/**
 * Connection base class.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/socket.h"
#include <string>
#include <iostream>


namespace c11httpd {


// Connection base class.
//
// This is the base class for both server listening sockets & client incoming sockets.
class conn_base_t {
public:
	conn_base_t(socket_t sd, const std::string& ip, uint16_t port, bool listening, bool ipv6)
		: m_ip(ip), m_sd(sd), m_port(port), m_listening(listening), m_ipv6(ipv6) {
	}

	virtual ~conn_base_t();
	virtual void close();

	socket_t sock() const {
		return this->m_sd;
	}

	void sock(socket_t sd) {
		this->m_sd = sd;
	}

	const std::string& ip() const {
		return this->m_ip;
	}

	void ip(const std::string& ip) {
		this->m_ip = ip;
	}

	uint16_t port() const {
		return this->m_port;
	}

	void port(uint16_t port) {
		this->m_port = port;
	}

	bool listening() const {
		return this->m_listening;
	}

	bool ipv6() const {
		return this->m_ipv6;
	}

	void ipv6(bool ipv6) {
		this->m_ipv6 = ipv6;
	}

private:
	// Remove default constructor, copy constructor, and operator=().
	conn_base_t() = delete;
	conn_base_t(const conn_base_t&) = delete;
	conn_base_t& operator=(const conn_base_t&) = delete;

private:
	std::string m_ip;
	socket_t m_sd;
	uint16_t m_port;
	const bool m_listening;
	bool m_ipv6;
};


template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(
		std::basic_ostream<Char, Traits>& ostream, const conn_base_t& conn) {
	ostream << conn.ip() << ":" << conn.port();

	return ostream;
}


}

