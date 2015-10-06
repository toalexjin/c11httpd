/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/acceptor.h"
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>


namespace c11httpd {

acceptor_t::~acceptor_t() {
	this->destroy();
}

void acceptor_t::destroy() {
	this->resize_bind_i(0);
}

err_t acceptor_t::bind(uint16_t port) {
	return this->bind(0, port);
}

err_t acceptor_t::bind(const char* ip, uint16_t port) {
	err_t ret;

	if (ip == 0 || *ip == 0) {
		ret = this->bind_ipv4(0, port);
		if (!ret) {
			return ret;
		}

		ret = this->bind_ipv6(0, port);
		if (!ret) {
			this->resize_bind_i(this->m_binds.size() - 1);
		}
	} else {
		if (std::strchr(ip, ':') == 0) {
			ret = this->bind_ipv4(ip, port);
		} else {
			ret = this->bind_ipv6(ip, port);
		}
	}

	return ret;
}

err_t acceptor_t::bind_ipv4(const char* ip, uint16_t port) {
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

	this->m_binds.push_back(fd);
	ret.set_ok();

clean:

	if (!ret) {
		fd.close();
	}

	return ret;
}

err_t acceptor_t::bind_ipv6(const char* ip, uint16_t port) {
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

	this->m_binds.push_back(fd);
	ret.set_ok();

clean:

	if (!ret) {
		fd.close();
	}

	return ret;
}

err_t acceptor_t::bind(std::initializer_list<std::pair<std::string, uint16_t>> list) {
	err_t ret;
	const size_t old_size = this->m_binds.size();

	for (auto it = list.begin(); it != list.end(); ++it) {
		ret = this->bind((*it).first.c_str(), (*it).second);

		if (!ret) {
			this->resize_bind_i(old_size);
		}
	}

	return ret;
}

err_t acceptor_t::accept() {
	err_t ret;
	int epoll = -1;
	struct epoll_event* events = new struct epoll_event[this->m_max_events];

	// Create epoll handle.
	epoll = epoll_create1(EPOLL_CLOEXEC);
	if (epoll == -1) {
		ret.set_current();
		goto clean;
	}

	// Add listening sockets.
	for (auto it = this->m_binds.cbegin(); it != this->m_binds.cend(); ++it) {
		ret = this->epoll_add_bind_i(epoll, (*it).get());
		if (!ret) {
			goto clean;
		}
	}

	while (true) {
		const int wait_result = epoll_wait(epoll, events, this->m_max_events, -1);
		if (wait_result == -1 && err_t::current() != EINTR) {
			ret.set_current();
			goto clean;
		}

		for (int i = 0; i < wait_result; ++i) {

		}
	}

	ret.set_ok();

clean:

	if (epoll != -1) {
		::close(epoll);
		epoll = -1;
	}

	delete[] events;
	events = 0;

	return ret;
}

void acceptor_t::resize_bind_i(size_t new_size) {
	assert(new_size <= this->m_binds.size());

	for (auto i = new_size; i < this->m_binds.size(); ++i) {
		this->m_binds[i].close();
	}
	this->m_binds.resize(new_size);
}

err_t acceptor_t::epoll_add_bind_i(int epoll, int new_fd) {
	struct epoll_event event;

	event.data.fd = new_fd;
	event.events = EPOLLIN | EPOLLET;

	return epoll_ctl(epoll, EPOLL_CTL_ADD, new_fd, &event);
}


} // namespace c11httpd.

