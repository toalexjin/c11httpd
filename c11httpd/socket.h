/**
 * Socket.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"


namespace c11httpd {

/**
 * A lightweight socket wrapper class.
 *
 * Note that this object will be saved in STL containers,
 * which might copy objects internally when re-allocating memory.
 * Therefore, this object will not close file handle automatically in destructor.
 */
class socket_t {
public:
	socket_t() : m_fd(-1) {
	}

	socket_t(int fd) : m_fd(fd) {
	}

	// We do not close file handle in destructor!!
	~socket_t() = default;

	socket_t(const socket_t&) = default;
	socket_t& operator=(const socket_t&) = default;

	socket_t& operator=(int fd) {
		return this->set(fd);
	}

	bool is_closed() const {
		return this->m_fd == -1;
	}

	int get() const {
		return this->m_fd;
	}

	socket_t& set(int fd) {
		this->m_fd = fd;
		return *this;
	}

	err_t close();
	err_t set_nonblock();

	err_t new_ipv4_nonblock();
	err_t new_ipv6_nonblock();

	err_t bind_ipv4(const char* ip, uint16_t port);
	err_t bind_ipv6(const char* ip, uint16_t port);
	err_t listen(int backlog);

private:
	int m_fd;
};

} // namespace c11httpd.


