/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/acceptor.h"
#include "c11httpd/conn.h"
#include "c11httpd/fd.h"
#include "c11httpd/http_processor.h"
#include "c11httpd/link.h"
#include "c11httpd/socket.h"
#include "c11httpd/waitable.h"
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/signalfd.h>


namespace c11httpd {


const std::string acceptor_t::ipv4_any("0.0.0.0");
const std::string acceptor_t::ipv4_loopback("127.0.0.1");

const std::string acceptor_t::ipv6_any("::");
const std::string acceptor_t::ipv6_loopback("::1");


acceptor_t::acceptor_t(const config_t& cfg)
	: m_config(cfg) {
	// Ignore SIGPIPE.
	signal(SIGPIPE, SIG_IGN);
}

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

	ret = sd.listen(this->m_config.backlog());
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

	ret = sd.listen(this->m_config.backlog());
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
	running_t running(handler);
	const int events_size = this->m_config.max_epoll_events();
	struct epoll_event* events = 0;
	std::set<conn_t*> aio_conns; // Which connection has completed AIO tasks.
	std::vector<aio_t> aio_completed; // Completed AIO tasks.
	socket_t new_sd;
	std::string new_ip;
	uint16_t new_port;
	bool new_ipv6;

	assert(handler != 0);

	// Create worker processes.
	if (this->m_config.worker_processes() > 0) {
		ret = this->m_worker_pool.create(this->m_config.worker_processes());
		if (!ret) {
			goto clean;
		}
	}

	// Create epoll handle.
	running.m_epoll = epoll_create1(EPOLL_CLOEXEC);
	if (!running.m_epoll.is_open()) {
		ret.set_current();
		goto clean;
	}

	// Hook Linux signals.
	ret = this->signalfd_i(&running.m_signal);
	if (!ret) {
		goto clean;
	}

	// Add signal fd to epoll.
	ret = this->epoll_set_i(running.m_epoll, running.m_signal.get(),
		&running.m_waitable_signal, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
	if (!ret) {
		goto clean;
	}

	// Add listening sockets.
	if (this->m_config.worker_processes() == 0 || !this->m_worker_pool.main_process()) {
		for (auto it = this->m_listens.begin(); it != this->m_listens.end(); ++it) {
			ret = this->epoll_set_i(running.m_epoll, (*it).get()->sock(), (*it).get(), EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
			if (!ret) {
				goto clean;
			}
		}
	}

	events = new struct epoll_event[events_size];
	while (true) {
		const int wait_result = epoll_wait(running.m_epoll.get(), events, events_size, -1);

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
			auto waitable = (const waitable_t*) events[i].data.ptr;

			if (waitable->wait_type() == waitable_t::type_signal) {
				bool exit;

				ret = this->on_signalled_i(running.m_epoll, running.m_signal, &exit, &aio_conns);
				if (!ret || exit) {
					goto clean;
				}

				for (conn_t* conn : aio_conns) {
					bool gc = false;

					// Popup AIO completed tasks.
					conn->popup_aio_completed(&aio_completed);
					if (aio_completed.empty()) {
						continue;
					}

					// Invoke callback function to handle completed aio tasks.
					conn->last_event_result(handler->on_aio_completed(
						*conn, this->m_config, *conn, aio_completed, conn->send_buf()));

					if (conn->pending_send_size() > 0) {
						this->epoll_set_i(running.m_epoll, conn->sock(), conn, EPOLL_CTL_MOD, EPOLLOUT | EPOLLET);
					} else if (conn->last_event_result() & conn_event_t::result_disconnect) {
						gc = true;
						break;
					}

					// An error happened or client side closed connection,
					// we need to garbage collect the conn.
					if (gc) {
						this->gc_conn_i(&running, conn, false);
					}
				}

				// Remove aio-completed conn pointers.
				aio_conns.clear();
			} else if (waitable->wait_type() == waitable_t::type_listen) {
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

					if (running.m_free_count > 0) {
						// Pop an free conn object from free list.
						running.m_free_count--;
						conn = running.m_free_list.pop_front()->get();
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
								*conn, this->m_config, *conn, conn->send_buf()));

						// If no data to send and disconnect flag is on,
						// then close connection.
						if ((conn->last_event_result() & conn_event_t::result_disconnect) != 0
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
						ret = this->epoll_set_i(running.m_epoll, conn->sock(), conn, EPOLL_CTL_ADD,
								conn->pending_send_size() == 0 ?
								(EPOLLIN | EPOLLET) : (EPOLLOUT | EPOLLET));

						if (!ret) {
							gc = true;
							break;
						}
					} while (0);

					if (gc) {
						this->gc_conn_i(&running, conn, true);
					} else {
						// Add it to used list.
						running.m_used_list.push_back(conn->link_node());
						running.m_used_count ++;
					}
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
								*conn, this->m_config, *conn,
								conn->recv_buf(), conn->send_buf()));
						}

						// Client side has closed connection.
						if (peer_closed) {
							gc = true;
							break;
						}

						if (conn->pending_send_size() > 0) {
							this->epoll_set_i(running.m_epoll, conn->sock(), conn, EPOLL_CTL_MOD, EPOLLOUT | EPOLLET);
						} else if (conn->last_event_result() & conn_event_t::result_disconnect) {
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
							if (conn->last_event_result() & conn_event_t::result_disconnect) {
								gc = true;
								break;
							}

							// All data has been sent, switch to receive data mode.
							this->epoll_set_i(running.m_epoll, conn->sock(), conn, EPOLL_CTL_MOD, EPOLLIN | EPOLLET);
						}
					}
				} while (0);

				// An error happened or client side closed connection,
				// we need to garbage collect the conn.
				if (gc) {
					this->gc_conn_i(&running, conn, false);
				}
			} else {
				// Should not run here!
				assert(false);
			}
		}
	}

	ret.set_ok();

clean:

	// Trigger "on_disconnected" event for each existing connection.
	do {
		const config_t* cfg = &this->m_config;
		running.m_used_list.for_each([handler, cfg](conn_t* c) {
			handler->on_disconnected(*c, *cfg, *c);
			delete c;
		});
	} while (0);

	running.m_aio_wait_list.clear();
	running.m_free_list.clear();

	if (events != 0) {
		delete[] events;
		events = 0;
	}

	running.m_epoll.close();
	running.m_signal.close();

	// Kill all worker process.
	this->m_worker_pool.kill_all();

	return ret;
}

err_t acceptor_t::run_tcp(const conn_event_adapter_t::on_received_t& recv) {
	conn_event_adapter_t adapter;

	adapter.lambda_on_received(recv);
	return this->run_tcp(&adapter);
}

err_t acceptor_t::run_http(rest_ctrl_t* controller) {
	assert(controller != 0);

	http_processor_t processor({controller});
	return this->run_tcp(&processor);
}

err_t acceptor_t::run_http(const std::vector<rest_ctrl_t*>& controllers) {
	http_processor_t processor(controllers);
	return this->run_tcp(&processor);
}

err_t acceptor_t::stop() {
	err_t ret;
	const auto pid = this->m_worker_pool.self_pid();

	if (pid > 0) {
		if (::kill(pid, SIGTERM) == -1) {
			ret.set_current();
		}
	}

	return ret;
}

err_t acceptor_t::epoll_set_i(fd_t epoll, socket_t sock,
	const waitable_t* waitable, int op, uint32_t events) {
	assert(epoll.is_open());
	assert(sock.is_open());
	assert(waitable != 0);

	struct epoll_event event;

	event.data.ptr = (void*) waitable;
	event.events = events;

	return epoll_ctl(epoll.get(), op, sock.get(), &event);
}

err_t acceptor_t::epoll_del_i(fd_t epoll, socket_t sock) {
	assert(epoll.is_open());
	assert(sock.is_open());

	return epoll_ctl(epoll.get(), EPOLL_CTL_DEL, sock.get(), 0);
}

void acceptor_t::add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn) {
	assert(!conn->link_node()->linked());

	if (*free_count < this->m_config.max_free_connection()) {
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

		if ((conn->last_event_result() & conn_event_t::result_more_data) == 0) {
			break;
		}

		conn->last_event_result(handler->get_more_data(
				*conn, this->m_config, *conn, conn->send_buf()));
		if (conn->pending_send_size() == 0) {
			assert((conn->last_event_result() & conn_event_t::result_more_data) == 0);
			break;
		}
	}

	return ret;
}

err_t acceptor_t::on_signalled_i(
	fd_t epoll, fd_t signal_fd, bool* exit, std::set<conn_t*>* aio_conns) {

	err_t ret;
	size_t new_read_size;
	bool eof;
	int dead_workers = 0;
	const struct signalfd_siginfo* ptr;

	assert(exit != 0);
	assert(aio_conns != 0);

	// Clear content.
	*exit = false;
	aio_conns->clear();

	// Read signal info.
	//
	// Note that we do NOT check return code immediately
	// because some signal info might have already been gotten
	// (e.g. AIO signals) that need to handle right away.
	ret = signal_fd.read_nonblock(&this->m_signal_buf, &new_read_size, &eof);

	ptr = (const struct signalfd_siginfo*) this->m_signal_buf.front();
	const size_t times = this->m_signal_buf.size() / sizeof(struct signalfd_siginfo);

	for (size_t i = 0; i < times; ++i) {
		if (ptr[i].ssi_signo == SIGINT || ptr[i].ssi_signo == SIGTERM) {
			*exit = true;
		} else if (ptr[i].ssi_signo == SIGCHLD) {
			dead_workers += this->on_worker_terminated_i();
		} else if (int(ptr[i].ssi_signo) == conn_t::aio_signal_id) {
			auto aio_node = (conn_t::aio_node_t*) ptr[i].ssi_ptr;
			if (aio_node != 0) {
				aio_node->m_conn->on_aio_completed_i(aio_node);
				aio_conns->insert(aio_node->m_conn);
			}
		} else {
			// Should not run here!
			assert(false);
		}
	}

	// Remove processed signal information from buf_t.
	this->m_signal_buf.erase_front(times * sizeof(struct signalfd_siginfo));

	// Restart dead worker processes.
	if (dead_workers > 0) {
		const err_t ret_tmp = this->restart_worker_i(epoll, dead_workers);
		if (!ret_tmp && ret.ok()) {
			ret = ret_tmp;
		}
	}

	return ret;
}

int acceptor_t::on_worker_terminated_i() {
	int dead_workers = 0;

	while (true) {
		int status;
		const auto pid = waitpid(-1, &status, WNOHANG);

		if (pid > 0) {
			if (this->m_worker_pool.on_terminated(pid)) {
				++ dead_workers;
			}
		} else {
			break;
		}
	}

	return dead_workers;
}

err_t acceptor_t::restart_worker_i(fd_t epoll, int dead_workers) {
	err_t ret;

	// Re-start worker processes if they died.
	if (dead_workers <= 0
		|| !this->m_worker_pool.main_process()
		|| this->m_config.worker_processes() <= 0) {
		return ret;
	}

	// Create worker processes.
	this->m_worker_pool.create(dead_workers);

	if (!this->m_worker_pool.main_process()) {
		// Add listening sockets.
		for (auto it = this->m_listens.begin(); it != this->m_listens.end(); ++it) {
			ret = this->epoll_set_i(epoll, (*it).get()->sock(), (*it).get(), EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
			if (!ret) {
				break;
			}
		}
	}

	return ret;
}

void acceptor_t::gc_conn_i(running_t* running, conn_t* conn, bool new_conn) {
	assert(running != 0);
	assert(conn != 0);

	if (new_conn) {
		// Trigger "on_disconnected" event.
		running->m_handler->on_disconnected(*conn, this->m_config, *conn);

		// Reset connection object.
		conn->close();

		// Put it to free list.
		this->add_free_conn_i(&running->m_free_list, &running->m_free_count, conn);
	} else if (conn->aio_wait_state()) {
		if (conn->aio_running_count() == 0) {
			// If there is no any running AIO tasks,
			// then remove it from aio_wait_list.
			conn->link_node()->unlink();
			running->m_aio_wait_count--;
			conn->aio_wait_state(false);

			// Reset connection object.
			conn->close();

			// Put it to free list.
			this->add_free_conn_i(&running->m_free_list, &running->m_free_count, conn);
		}
	} else if (conn->aio_running_count() > 0) {
		// Trigger "on_disconnected" event.
		running->m_handler->on_disconnected(*conn, this->m_config, *conn);

		// Remove it from used list.
		conn->link_node()->unlink();
		running->m_used_count--;

		// Reset connection object.
		conn->close();

		// As there are pending AIO tasks, put it to aio_wait_list.
		conn->aio_wait_state(true);
		running->m_aio_wait_list.push_back(conn->link_node());
		running->m_aio_wait_count++;
	} else {
		// Trigger "on_disconnected" event.
		running->m_handler->on_disconnected(*conn, this->m_config, *conn);

		// Remove it from used list.
		conn->link_node()->unlink();
		running->m_used_count--;

		// Reset connection object.
		conn->close();

		// Put it to free list.
		this->add_free_conn_i(&running->m_free_list, &running->m_free_count, conn);
	}
}

err_t acceptor_t::signalfd_i(fd_t* fd) {
	sigset_t signal_mask;

	assert(fd != 0);

	*fd = -1;

	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGTERM);
	sigaddset(&signal_mask, SIGINT);
	sigaddset(&signal_mask, SIGCHLD);
	sigaddset(&signal_mask, conn_t::aio_signal_id);

	/* Block the signals that we handle using signalfd(), so they don't
	 * cause signal handlers or default signal actions to execute. */
	if (sigprocmask(SIG_BLOCK, &signal_mask, 0) == -1) {
		return err_t::current();
	}

	// Hook Linux signal events.
	*fd = signalfd(-1, &signal_mask, SFD_NONBLOCK | SFD_CLOEXEC);
	if (!fd->is_open()) {
		return err_t::current();
	}

	return err_t();
}

} // namespace c11httpd.

