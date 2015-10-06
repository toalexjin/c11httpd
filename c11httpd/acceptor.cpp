/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/acceptor.h"
#include "c11httpd/conn.h"
#include "c11httpd/fd.h"
#include "c11httpd/socket.h"
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>


namespace c11httpd {

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
		ret = this->bind_ipv4(ip, port);
		if (!ret) {
			return ret;
		}

		ret = this->bind_ipv6(ip, port);
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

	ret = sd.new_ipv4_nonblock();
	if (!ret) {
		goto clean;
	}

	ret = sd.bind_ipv4(ip, port);
	if (!ret) {
		goto clean;
	}

	ret = sd.listen(this->m_backlog);
	if (!ret) {
		goto clean;
	}

	this->m_listens.emplace_back(new conn_base_t(sd, ip, port, true, false));
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

	ret = sd.new_ipv6_nonblock();
	if (!ret) {
		goto clean;
	}

	ret = sd.bind_ipv6(ip, port);
	if (!ret) {
		goto clean;
	}

	ret = sd.listen(this->m_backlog);
	if (!ret) {
		goto clean;
	}

	this->m_listens.emplace_back(new conn_base_t(sd, ip, port, true, true));
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
		}
	}

	return ret;
}

err_t acceptor_t::run() {
	err_t ret;
	fd_t epoll;
	struct epoll_event* events = new struct epoll_event[this->m_max_events];
	socket_t new_sd;
	std::string new_ip;
	uint16_t new_port;
	bool new_ipv6;
	size_t new_data_size;

	// Create epoll handle.
	epoll = epoll_create1(EPOLL_CLOEXEC);
	if (!epoll.is_opened()) {
		ret.set_current();
		goto clean;
	}

	// Add listening sockets.
	for (auto it = this->m_listens.begin(); it != this->m_listens.end(); ++it) {
		ret = this->epoll_add_i(epoll, (*it).get());
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
			auto base = (conn_base_t*) events[i].data.ptr;

			if (base->is_listening()) {
				// New connections are coming.
				while (true) {
					ret = base->get_socket().accept(&new_sd, &new_ip, &new_port, &new_ipv6);
					if (!ret) {
						if (ret == EAGAIN || ret == EWOULDBLOCK) {
							break;
						} else {
							goto clean;
						}
					}

					// Add the new connection to epoll.
					auto new_conn = new conn_t(new_sd, new_ip, new_port, new_ipv6);
					this->epoll_add_i(epoll, new_conn);
				}

			} else {
				// New data arrived.
				auto conn = (conn_t*) base;
				ret = conn->recv(&new_data_size);

				// An error happened.
				if (!ret) {
					this->epoll_del_i(epoll, conn);
					delete conn;
					continue;
				}

				// Client side closed connection.
				if (new_data_size == 0) {
					this->epoll_del_i(epoll, conn);
					delete conn;
				}
			}
		}
	}

	ret.set_ok();

clean:

	delete[] events;
	events = 0;

	epoll.close();
	return ret;
}

err_t acceptor_t::epoll_add_i(fd_t epoll, conn_base_t* conn) {
	assert(epoll.is_opened());
	assert(conn != 0);

	struct epoll_event event;

	event.data.ptr = conn;
	event.events = EPOLLIN | EPOLLET;

	return epoll_ctl(epoll.get(), EPOLL_CTL_ADD, conn->get_socket().get(), &event);
}

err_t acceptor_t::epoll_del_i(fd_t epoll, conn_base_t* conn) {
	assert(epoll.is_opened());
	assert(conn != 0);

	return epoll_ctl(epoll.get(), EPOLL_CTL_DEL, conn->get_socket().get(), 0);
}


} // namespace c11httpd.

