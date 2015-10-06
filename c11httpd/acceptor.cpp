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
	fd_t epoll;
	struct epoll_event* events = new struct epoll_event[this->m_max_events];

	// Create epoll handle.
	epoll = epoll_create1(EPOLL_CLOEXEC);
	if (epoll.is_closed()) {
		ret.set_current();
		goto clean;
	}

	// Add listening sockets.
	for (auto it = this->m_binds.begin(); it != this->m_binds.end(); ++it) {
		ret = this->epoll_add_bind_i(epoll, (*it));
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

void acceptor_t::resize_bind_i(size_t new_size) {
	assert(new_size <= this->m_binds.size());

	for (auto i = new_size; i < this->m_binds.size(); ++i) {
		this->m_binds[i].close();
	}
	this->m_binds.resize(new_size);
}

err_t acceptor_t::epoll_add_bind_i(fd_t& epoll, socket_t& new_fd) {
	assert(epoll.is_opened());
	assert(new_fd.is_opened());

	struct epoll_event event;

	event.data.fd = new_fd.get();
	event.events = EPOLLIN | EPOLLET;

	return epoll_ctl(epoll.get(), EPOLL_CTL_ADD, new_fd.get(), &event);
}


} // namespace c11httpd.

