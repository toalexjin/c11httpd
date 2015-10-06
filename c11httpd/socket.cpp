/**
 * Socket.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/socket.h"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


namespace c11httpd {


err_t socket_t::new_ipv4_nonblock() {
	this->set(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
	if (this->get() == -1) {
		return err_t::current();
	}

	return err_t();
}

err_t socket_t::new_ipv6_nonblock() {
	this->set(socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
	if (this->get() == -1) {
		return err_t::current();
	}

	return err_t();
}

err_t socket_t::bind_ipv4(const std::string& ip, uint16_t port) {
	assert(!this->is_closed());

	struct sockaddr_in address;

	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (!ip.empty()) {
		const int result = inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
		if (result != 1) {
			if (result == 0) {
				return EINVAL;
			} else {
				return err_t::current();
			}
		}
	}

	if (bind(this->get(), (struct sockaddr*) &address, sizeof(address)) != 0) {
		return err_t::current();
	}

	return err_t();
}

err_t socket_t::bind_ipv6(const std::string& ip, uint16_t port) {
	assert(!this->is_closed());

	struct sockaddr_in6 address;

	bzero(&address, sizeof(address));
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(port);

	if (!ip.empty()) {
		const int result = inet_pton(AF_INET6, ip.c_str(), &address.sin6_addr);
		if (result != 1) {
			if (result == 0) {
				return EINVAL;
			} else {
				return err_t::current();
			}
		}
	}

	if (bind(this->get(), (struct sockaddr*) &address, sizeof(address)) != 0) {
		return err_t::current();
	}

	return err_t();
}

err_t socket_t::accept(socket_t* sd, std::string* ip, uint16_t* port, bool* ipv6) {
	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	char buf[INET6_ADDRSTRLEN + 1];

	assert(this->is_opened());
	assert(sd != 0);
	assert(ip != 0);
	assert(port != 0);
	assert(ipv6 != 0);

	const auto result = ::accept(this->get(), (struct sockaddr*) &addr, &len);
	if (result < 0) {
		return err_t::current();
	}

	// Set return values.
	if (addr.ss_family == AF_INET) {
		const auto addr_v4 = (struct sockaddr_in*) &addr;

		if (inet_ntop(AF_INET, &addr_v4->sin_addr, buf, sizeof(buf)) == 0) {
			const auto e = err_t::current();
			::close(result);
			return e;
		}

		*ip = buf;
		*port = ntohs(addr_v4->sin_port);
		*ipv6 = false;

	} else if (addr.ss_family == AF_INET6) {
		const auto addr_v6 = (struct sockaddr_in6*) &addr;

		if (inet_ntop(AF_INET6, &addr_v6->sin6_addr, buf, sizeof(buf)) == 0) {
			const auto e = err_t::current();
			::close(result);
			return e;
		}

		*ip = buf;
		*port = ntohs(addr_v6->sin6_port);
		*ipv6 = true;

	} else {
		::close(result);
		return EINVAL;
	}

	*sd = result;
	return err_t();
}

err_t socket_t::listen(int backlog) {
	assert(!this->is_closed());

	if (::listen(this->get(), backlog) != 0) {
		return err_t::current();
	}

	return err_t();
}

err_t socket_t::set_nonblock() {
	err_t ret;

	assert(!this->is_closed());

	const auto old = fcntl(this->get(), F_GETFL);
	if (fcntl(this->get(), F_SETFL, old | O_NONBLOCK) != 0) {
		ret.set_current();
	}

	return ret;
}


} // namespace c11httpd.


