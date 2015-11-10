/**
 * Client session.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include "c11httpd/fd.h"
#include <string>
#include <vector>


namespace c11httpd {


// Aio task.
class aio_t {
public:
	int64_t m_id;
	err_t m_error;
	fd_t m_fd;
	int64_t m_offset;
	char* m_buf;
	size_t m_size;
	size_t m_ok_bytes;
};


// Client session.
//
// This is client session interface, implemented by conn_t.
class conn_session_t {
public:
	conn_session_t() = default;
	virtual ~conn_session_t() = default;

	virtual const std::string& ip() const = 0;
	virtual uint16_t port() const = 0;
	virtual bool ipv6() const = 0;

	// AIO operations.
	virtual err_t aio_read(fd_t fd, int64_t offset, char* buf, size_t size, int64_t* id = 0) = 0;
	virtual err_t aio_write(fd_t fd, int64_t offset, const char* buf, size_t size, int64_t* id = 0) = 0;
	virtual err_t aio_cancel(fd_t fd) = 0;
};


template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(
		std::basic_ostream<Char, Traits>& ostream, const conn_session_t& s) {
	ostream << s.ip() << ":" << s.port() << "(" << (const void*) &s << ")";

	return ostream;
}


}

