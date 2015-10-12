/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/acceptor.h"
#include "c11httpd/conn.h"
#include "c11httpd/fd.h"
#include "c11httpd/link.h"
#include "c11httpd/socket.h"
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>


namespace c11httpd {


const std::string acceptor_t::ipv4_any("0.0.0.0");
const std::string acceptor_t::ipv4_loopback("127.0.0.1");

const std::string acceptor_t::ipv6_any("::");
const std::string acceptor_t::ipv6_loopback("::1");


acceptor_t::~acceptor_t() {
	this->close();
}

void acceptor_t::close() {
	this->m_listens.clear();
}

err_t acceptor_t::bind(uint16_t port) {
	return this->bind(std::string(), port);
}

err_t acceptor_t::bind(const std::string& ip, uint16_t port) {
	err_t ret;

	if (ip.empty()) {
		ret = this->bind_ipv4(ipv4_any, port);
		if (!ret) {
			return ret;
		}

		ret = this->bind_ipv6(ipv6_any, port);
		if (!ret) {
			this->m_listens.resize(this->m_listens.size() - 1);
		}
	} else {
		if (ip.find(':') == std::string::npos) {
			ret = this->bind_ipv4(ip, port);
		} else {
			ret = this->bind_ipv6(ip, port);
		}
	}

	return ret;
}

err_t acceptor_t::bind_ipv4(const std::string& ip, uint16_t port) {
	err_t ret;
	socket_t sd;
	const std::string& real_ip = ip.empty() ? ipv4_any : ip;

	ret = sd.new_ipv4_nonblock();
	if (!ret) {
		goto clean;
	}

	ret = sd.bind_ipv4(real_ip, port);
	if (!ret) {
		goto clean;
	}

	ret = sd.listen(this->m_backlog);
	if (!ret) {
		goto clean;
	}

	this->m_listens.emplace_back(new listen_t(sd, real_ip, port, false));
	ret.set_ok();

clean:

	if (!ret) {
		sd.close();
	}

	return ret;
}

err_t acceptor_t::bind_ipv6(const std::string& ip, uint16_t port) {
	err_t ret;
	socket_t sd;
	const std::string& real_ip = ip.empty() ? ipv6_any : ip;

	ret = sd.new_ipv6_nonblock();
	if (!ret) {
		goto clean;
	}

	ret = sd.bind_ipv6(real_ip, port);
	if (!ret) {
		goto clean;
	}

	ret = sd.listen(this->m_backlog);
	if (!ret) {
		goto clean;
	}

	this->m_listens.emplace_back(new listen_t(sd, real_ip, port, true));
	ret.set_ok();

clean:

	if (!ret) {
		sd.close();
	}

	return ret;
}

err_t acceptor_t::bind(std::initializer_list<std::pair<std::string, uint16_t>> list) {
	err_t ret;
	const size_t old_size = this->m_listens.size();

	for (auto it = list.begin(); it != list.end(); ++it) {
		ret = this->bind((*it).first.c_str(), (*it).second);

		if (!ret) {
			this->m_listens.resize(old_size);
			return ret;
		}
	}

	return ret;
}

std::vector<std::pair<std::string, uint16_t>> acceptor_t::binds() const {
	std::vector<std::pair<std::string, uint16_t>> vt;

	vt.reserve(this->m_listens.size());
	for (auto it = this->m_listens.cbegin(); it != this->m_listens.cend(); ++it) {
		vt.push_back(std::pair<std::string, uint16_t>((*it)->ip(), (*it)->port()));
	}

	return vt;
}

err_t acceptor_t::run_tcp(conn_event_t* handler) {
	err_t ret;
	link_t<conn_t> used_list;
	link_t<conn_t> free_list;
	int used_count = 0;
	int free_count = 0;
	fd_t epoll;
	struct epoll_event* events = new struct epoll_event[this->m_max_events];
	socket_t new_sd;
	std::string new_ip;
	uint16_t new_port;
	bool new_ipv6;

	assert(handler != 0);

	// Create epoll handle.
	epoll = epoll_create1(EPOLL_CLOEXEC);
	if (!epoll.opened()) {
		ret.set_current();
		goto clean;
	}

	// Add listening sockets.
	for (auto it = this->m_listens.begin(); it != this->m_listens.end(); ++it) {
		ret = this->epoll_set_i(epoll, (*it).get(), EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
		if (!ret) {
			goto clean;
		}
	}

	while (true) {
		const int wait_result = epoll_wait(epoll.get(), events, this->m_max_events, -1);

		if (wait_result == -1) {
			const auto e = err_t::current();

			if (e == EINTR) {
				continue;
			} else {
				ret.set(e);
				goto clean;
			}
		}

		for (int i = 0; i < wait_result; ++i) {
			auto waitable = (waitable_t*) events[i].data.ptr;

			if (waitable->wait_type() == waitable_t::type_listen) {
				auto listen = (listen_t*) waitable;

				while (true) {
					conn_t* conn;
					bool gc = false;

					// Accept new connection.
					ret = listen->sock().accept(&new_sd, &new_ip, &new_port, &new_ipv6);
					if (!ret) {
						if (ret == EAGAIN || ret == EWOULDBLOCK) {
							break;
						} else {
							goto clean;
						}
					}

					// Set non-block flag.
					ret = new_sd.nonblock(true);
					if (!ret) {
						new_sd.close();
						continue;
					}

					if (free_count > 0) {
						// Pop an free conn object from free list.
						--free_count;
						conn = free_list.pop_front()->get();
						conn->sock(new_sd);
						conn->ip(new_ip);
						conn->port(new_port);
						conn->ipv6(new_ipv6);
					} else {
						// Create a new conn object.
						conn = new conn_t(new_sd, new_ip, new_port, new_ipv6);
					}

					do {
						// Trigger "on_connected" event.
						conn->last_event_result(handler->on_connected(
								conn, conn->send_buf()));

						// If no data to send and disconnect flag is on,
						// then close connection.
						if ((conn->last_event_result() & event_result_disconnect) != 0
							&& conn->pending_send_size() == 0) {
							gc = true;
							break;
						}

						// If there are data to send, then send it right now.
						if (conn->pending_send_size() > 0) {
							ret = this->loop_send_i(handler, conn);
							if (!ret) {
								gc = true;
								break;
							}
						}

						// Add it to epoll list.
						ret = this->epoll_set_i(epoll, conn, EPOLL_CTL_ADD,
								conn->pending_send_size() == 0 ?
								(EPOLLIN | EPOLLET) : (EPOLLOUT | EPOLLET));

						if (!ret) {
							gc = true;
							break;
						}
					} while (0);

					if (gc) {
						// Trigger "on_disconnected" event.
						handler->on_disconnected(conn);

						this->add_free_conn_i(&free_list, &free_count, conn);
						conn = 0;
						continue;
					}

					// Add it to used list.
					used_list.push_back(conn->link_node());
					++ used_count;
				}
			} else if (waitable->wait_type() == waitable_t::type_conn) {
				auto conn = (conn_t*) waitable;
				bool gc = false;

				do {
					if (events[i].events & EPOLLIN) {
						size_t new_recv_size;
						bool peer_closed;

						// New data is ready to read.
						ret = conn->recv(&new_recv_size, &peer_closed);
						if (!ret) {
							gc = true;
							break;
						}

						// Trigger "on_received" event.
						if (new_recv_size > 0) {
							conn->last_event_result(handler->on_received(
									conn, conn->recv_buf(), conn->send_buf()));
						}

						// Client side has closed connection.
						if (peer_closed) {
							gc = true;
							break;
						}

						if (conn->pending_send_size() > 0) {
							this->epoll_set_i(epoll, conn, EPOLL_CTL_MOD, EPOLLOUT | EPOLLET);
						} else if (conn->last_event_result() & event_result_disconnect) {
							gc = true;
							break;
						}
					} else if (events[i].events & EPOLLOUT) {
						ret = this->loop_send_i(handler, conn);
						if (!ret) {
							gc = true;
							break;
						}

						if (conn->pending_send_size() == 0) {
							// If all data has been sent and disconnect flag is on,
							// then close connection.
							if (conn->last_event_result() & event_result_disconnect) {
								gc = true;
								break;
							}

							// All data has been sent, switch to receive data mode.
							this->epoll_set_i(epoll, conn, EPOLL_CTL_MOD, EPOLLIN | EPOLLET);
						}
					}
				} while (0);

				// An error happened or client side closed connection,
				// we need to garbage collect the conn.
				if (gc) {
					if (this->epoll_del_i(epoll, conn).ok()) {
						// Trigger "on_disconnected" event.
						handler->on_disconnected(conn);

						// Remove it from used list.
						conn->link_node()->unlink();
						-- used_count;

						// Add it to free list.
						this->add_free_conn_i(&free_list, &free_count, conn);
						conn = 0;
					}
				}
			} else if (waitable->wait_type() == waitable_t::type_signal) {
				;
			} else {
				assert(false);
			}
		}
	}

	ret.set_ok();

clean:

	// Trigger "on_disconnected" event for each existing connection.
	used_list.for_each([handler](conn_t* c) {
		handler->on_disconnected(c);
		delete c;
	});

	free_list.clear();

	delete[] events;
	events = 0;

	epoll.close();

	return ret;
}

err_t acceptor_t::epoll_set_i(fd_t epoll, waitable_t* waitable, int op, uint32_t events) {
	assert(epoll.opened());
	assert(waitable != 0);

	struct epoll_event event;

	event.data.ptr = waitable;
	event.events = events;

	return epoll_ctl(epoll.get(), op, waitable->fd(), &event);
}

err_t acceptor_t::epoll_del_i(fd_t epoll, waitable_t* waitable) {
	assert(epoll.opened());
	assert(waitable != 0);

	return epoll_ctl(epoll.get(), EPOLL_CTL_DEL, waitable->fd(), 0);
}

void acceptor_t::add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn) {
	assert(!conn->link_node()->linked());

	conn->close();

	if (*free_count < this->m_max_free_conn) {
		free_list->push_front(conn->link_node());
		++ *free_count;
	} else {
		delete conn;
	}
}

err_t acceptor_t::loop_send_i(conn_event_t* handler, conn_t* conn) {
	err_t ret;

	while (true) {
		size_t new_send_size;
		ret = conn->send(&new_send_size);
		if (!ret) {
			if (ret == EAGAIN || ret == EWOULDBLOCK) {
				ret.set_ok();
			}

			break;
		}

		if ((conn->last_event_result() & event_result_more_data) == 0) {
			break;
		}

		conn->last_event_result(handler->get_more_data(conn, conn->send_buf()));
		if (conn->pending_send_size() == 0) {
			assert((conn->last_event_result() & event_result_more_data) == 0);
			break;
		}
	}

	return ret;
}

} // namespace c11httpd.

