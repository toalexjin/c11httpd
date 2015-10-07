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

	socket_t(int sd) : fd_t(sd) {
	}

	socket_t(const socket_t& another) : fd_t(another) {
	}

	// We do not close file handle in destructor!!
	~socket_t() = default;

	socket_t& operator=(const socket_t& another) {
		fd_t::operator=(another);
		return *this;
	}

	socket_t& operator=(int sd) {
		return this->set(sd);
	}

	// socket_t::set() is often used along with Linux system APIs,
	// so it's very important to keep "errno" no change
	// after this function returns.
	socket_t& set(int sd) {
		fd_t::set(sd);
		return *this;
	}

	err_t new_ipv4_nonblock();
	err_t new_ipv6_nonblock();

	err_t bind_ipv4(const std::string& ip, uint16_t port);
	err_t bind_ipv6(const std::string& ip, uint16_t port);

	err_t accept(socket_t* sd, std::string* ip, uint16_t* port, bool* ipv6);
	err_t listen(int backlog);

	err_t send(const void* buf, size_t size, size_t* ok_bytes);
	err_t recv(void* buf, size_t size, size_t* ok_bytes);

	bool nonblock() const;
	err_t nonblock(bool flag);
};

} // namespace c11httpd.


