/**
 * Listening socket.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/socket.h"
#include "c11httpd/waitable.h"
#include <string>
#include <iostream>


namespace c11httpd {


// Listening socket.
class listen_t : public waitable_t {
public:
	listen_t(socket_t sd, const std::string& ip, uint16_t port, bool ipv6)
		: waitable_t(waitable_t::type_listen), m_ip(ip),
		  m_sd(sd), m_port(port), m_ipv6(ipv6) {
	}

	virtual ~listen_t();
	virtual void close();

	// Get file descriptor of the socket.
	virtual int fd() const;

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

	bool ipv6() const {
		return this->m_ipv6;
	}

	void ipv6(bool ipv6) {
		this->m_ipv6 = ipv6;
	}

private:
	// Remove default constructor, copy constructor, and operator=().
	listen_t() = delete;
	listen_t(const listen_t&) = delete;
	listen_t& operator=(const listen_t&) = delete;

private:
	std::string m_ip;
	socket_t m_sd;
	uint16_t m_port;
	bool m_ipv6;
};


template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(
		std::basic_ostream<Char, Traits>& ostream, const listen_t& listen) {
	ostream << listen.ip() << ":" << listen.port();

	return ostream;
}


}

