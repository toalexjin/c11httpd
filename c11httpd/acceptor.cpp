/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/acceptor.h"
#include "c11httpd/fd.h"
#include "c11httpd/socket.h"
#include <cstring>
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
	socket_t fd;

	ret = fd.new_ipv4_nonblock();
	if (!ret) {
		goto clean;
	}

	ret = fd.bind_ipv4(ip, port);
	if (!ret) {
		goto clean;
	}

	ret = fd.listen(this->m_backlog);
	if (!ret) {
		goto clean;
	}

	this->m_listens.emplace_back(new conn_base_t(fd, ip, port, true, false));
	ret.set_ok();

clean:

	if (!ret) {
		fd.close();
	}

	return ret;
}

err_t acceptor_t::bind_ipv6(const std::string& ip, uint16_t port) {
	err_t ret;
	socket_t fd;

	ret = fd.new_ipv6_nonblock();
	if (!ret) {
		goto clean;
	}

	ret = fd.bind_ipv6(ip, port);
	if (!ret) {
		goto clean;
	}

	ret = fd.listen(this->m_backlog);
	if (!ret) {
		goto clean;
	}

	this->m_listens.emplace_back(new conn_base_t(fd, ip, port, true, true));
	ret.set_ok();

clean:

	if (!ret) {
		fd.close();
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

err_t acceptor_t::accept() {
	err_t ret;
	fd_t epoll;
	struct epoll_event* events = new struct epoll_event[this->m_max_events];

	// Create epoll handle.
	epoll = epoll_create1(EPOLL_CLOEXEC);
	if (epoll.is_closed()) {
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

		}
	}

	ret.set_ok();

clean:

	delete[] events;
	events = 0;

	epoll.close();
	return ret;
}

err_t acceptor_t::epoll_add_i(fd_t epoll, conn_base_t* new_conn) {
	assert(epoll.is_opened());
	assert(new_conn != 0);

	struct epoll_event event;

	event.data.ptr = new_conn;
	event.events = EPOLLIN | EPOLLET;

	return epoll_ctl(epoll.get(), EPOLL_CTL_ADD, new_conn->get_socket().get(), &event);
}


} // namespace c11httpd.

