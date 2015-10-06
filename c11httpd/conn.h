/**
 * TCP connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/conn_base.h"
#include "c11httpd/socket.h"
#include <string>


namespace c11httpd {


/**
 * TCP connection.
 */
class conn_t : public conn_base_t {
public:
	conn_t(const socket_t& fd, const std::string& ip, uint16_t port, bool ipv6)
		: conn_base_t(fd, ip, port, false, ipv6) {
	}

	virtual ~conn_t();
	virtual void close();

private:
	// Remove default constructor, copy constructor and operator=().
	conn_t() = delete;
	conn_t(const conn_t&) = delete;
	conn_t& operator=(const conn_t&) = delete;

private:
	buf_t m_recv;
	buf_t m_send;
};


}

