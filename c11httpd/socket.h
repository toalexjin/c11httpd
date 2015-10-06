/**
 * A lightweight socket wrapper class.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include "c11httpd/fd.h"


namespace c11httpd {

/**
 * A lightweight socket wrapper class.
 *
 * Note that this object will be saved in STL containers,
 * which might copy objects internally when re-allocating memory.
 * Therefore, this object will not close file handle automatically in destructor.
 */
class socket_t : public fd_t {
public:
	socket_t() : fd_t() {
	}

	socket_t(int handle) : fd_t(handle) {
	}

	socket_t(const socket_t& another) : fd_t(another) {
	}

	// We do not close file handle in destructor!!
	~socket_t() = default;

	socket_t& operator=(const socket_t& another) {
		fd_t::operator=(another);
		return *this;
	}

	socket_t& operator=(int handle) {
		return this->set(handle);
	}

	socket_t& set(int handle) {
		fd_t::set(handle);
		return *this;
	}

	err_t set_nonblock();

	err_t new_ipv4_nonblock();
	err_t new_ipv6_nonblock();

	err_t bind_ipv4(const char* ip, uint16_t port);
	err_t bind_ipv6(const char* ip, uint16_t port);
	err_t listen(int backlog);
};

} // namespace c11httpd.


