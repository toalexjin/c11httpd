/**
 * TCP listener.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/listener.h"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


namespace c11httpd {

listener_t::~listener_t() {
	this->destroy();
}

void listener_t::destroy() {
	this->resize_i(0);
}

err_t listener_t::bind(uint16_t port) {
	return this->bind(0, port);
}

err_t listener_t::bind(const char* ip, uint16_t port) {
	err_t ret;

	if (ip == 0 || *ip == 0) {
		ret = this->bind_ipv4(0, port);
		if (!ret) {
			return ret;
		}

		ret = this->bind_ipv6(0, port);
		if (!ret) {
			this->resize_i(this->m_binds.size() - 1);
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

err_t listener_t::bind_ipv4(const char* ip, uint16_t port) {
	err_t ret;
	int fd = -1;
	struct sockaddr_in address;

	// Create address structure.
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (ip != 0 && *ip != 0) {
		const int result = inet_pton(AF_INET, ip, &address.sin_addr);
		if (result != 1) {
			if (result == 0) {
				ret.set(EINVAL);
			} else {
				ret.set_current();
			}

			goto clean;
		}
	}

	// Create socket.
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd == -1) {
		ret.set_current();
		goto clean;
	}

	if (::bind(fd, (struct sockaddr*) &address, sizeof(address)) != 0) {
		ret.set_current();
		goto clean;
	}

	if (::listen(fd, 10) != 0) {
		ret.set_current();
		goto clean;
	}

	this->m_binds.push_back(fd);
	ret.set_ok();

clean:

	if (!ret) {
		if (fd != -1) {
			close(fd);
		}
	}

	return ret;
}

err_t listener_t::bind_ipv6(const char* ip, uint16_t port) {
	err_t ret;
	int fd = -1;
	struct sockaddr_in6 address;

	// Create address structure.
	bzero(&address, sizeof(address));
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(port);

	if (ip != 0 && *ip != 0) {
		const int result = inet_pton(AF_INET6, ip, &address.sin6_addr);
		if (result != 1) {
			if (result == 0) {
				ret.set(EINVAL);
			} else {
				ret.set_current();
			}

			goto clean;
		}
	}

	// Create socket.
	fd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd == -1) {
		ret.set_current();
		goto clean;
	}

	if (::bind(fd, (struct sockaddr*) &address, sizeof(address)) != 0) {
		ret.set_current();
		goto clean;
	}

	if (::listen(fd, 10) != 0) {
		ret.set_current();
		goto clean;
	}

	this->m_binds.push_back(fd);
	ret.set_ok();

clean:

	if (!ret) {
		if (fd != -1) {
			close(fd);
		}
	}

	return ret;
}

err_t listener_t::bind(std::initializer_list<std::pair<std::string, uint16_t>> list) {
	err_t ret;
	const size_t old_size = this->m_binds.size();

	for (auto it = list.begin(); it != list.end(); ++it) {
		ret = this->bind((*it).first.c_str(), (*it).second);

		if (!ret) {
			this->resize_i(old_size);
		}
	}

	return ret;
}

void listener_t::resize_i(size_t new_size) {
	assert(new_size <= this->m_binds.size());

	for (auto i = new_size; i < this->m_binds.size(); ++i) {
		close(this->m_binds[i]);
	}
	this->m_binds.resize(new_size);
}


} // namespace c11httpd.

