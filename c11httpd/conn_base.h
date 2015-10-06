/**
 * Connection base class.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/socket.h"
#include <string>


namespace c11httpd {


/**
 * Connection base class.
 */
class conn_base_t {
public:
	conn_base_t(socket_t fd, const std::string& ip, uint16_t port, bool listening, bool ipv6)
		: m_fd(fd), m_ip(ip), m_port(port), m_listening(listening), m_ipv6(ipv6) {
	}

	virtual ~conn_base_t();
	virtual void close();

	socket_t get_socket() const {
		return this->m_fd;
	}

	void set_socket(socket_t fd) {
		this->m_fd = fd;
	}

	const std::string& get_ip() const {
		return this->m_ip;
	}

	void set_ip(const std::string& ip) {
		this->m_ip = ip;
	}

	uint16_t get_port() const {
		return this->m_port;
	}

	void set_port(uint16_t port) {
		this->m_port = port;
	}

	bool is_listening() const {
		return this->m_listening;
	}

	bool is_ipv6() const {
		return this->m_ipv6;
	}

	void set_ipv6(bool ipv6) {
		this->m_ipv6 = ipv6;
	}

private:
	// Remove default constructor, copy constructor, and operator=().
	conn_base_t() = delete;
	conn_base_t(const conn_base_t&) = delete;
	conn_base_t& operator=(const conn_base_t&) = delete;

private:
	socket_t m_fd;
	std::string m_ip;
	uint16_t m_port;
	const bool m_listening;
	bool m_ipv6;
};


}

