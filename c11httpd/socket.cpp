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


err_t socket_t::set_nonblock() {
	err_t ret;

	assert(!this->is_closed());

	const auto old = fcntl(this->get(), F_GETFL);
	if (fcntl(this->get(), F_SETFL, old | O_NONBLOCK) != 0) {
		ret.set_current();
	}

	return ret;
}

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

err_t socket_t::bind_ipv4(const char* ip, uint16_t port) {
	assert(!this->is_closed());

	struct sockaddr_in address;

	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (ip != 0 && *ip != 0) {
		const int result = inet_pton(AF_INET, ip, &address.sin_addr);
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

err_t socket_t::bind_ipv6(const char* ip, uint16_t port) {
	assert(!this->is_closed());

	struct sockaddr_in6 address;

	bzero(&address, sizeof(address));
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(port);

	if (ip != 0 && *ip != 0) {
		const int result = inet_pton(AF_INET6, ip, &address.sin6_addr);
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

err_t socket_t::listen(int backlog) {
	assert(!this->is_closed());

	if (::listen(this->get(), backlog) != 0) {
		return err_t::current();
	}

	return err_t();
}


} // namespace c11httpd.


