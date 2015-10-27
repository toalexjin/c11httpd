/**
 * Client connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/conn_session.h"
#include "c11httpd/link.h"
#include "c11httpd/listen.h"
#include "c11httpd/socket.h"
#include <string>


namespace c11httpd {


// Client connection.
//
// For each new client incoming connection, a conn_t object
// would be created. After the client connection was disconnected,
// the conn_t object might be re-used by acceptor_t for better performance.
class conn_t : public waitable_t, public conn_session_t {
public:
	conn_t(const socket_t& sd, const std::string& ip, uint16_t port, bool ipv6)
		: waitable_t(waitable_t::type_conn),
		m_ip(ip), m_sd(sd), m_port(port), m_ipv6(ipv6),
		m_link(uintptr_t(&this->m_link) - uintptr_t(this)) {

		assert(this == this->m_link.get());
		this->m_send_offset = 0;
		this->m_last_event_result = 0;
	}

	virtual ~conn_t();

	// Close handles, reset variables, but do not free memory.
	//
	// acceptor_t would call conn_t::close() and then
	// put the object to a free conn_t list for re-use.
	virtual void close();

	socket_t sock() const {
		return this->m_sd;
	}

	void sock(socket_t sd) {
		this->m_sd = sd;
	}

	void ip(const std::string& ip) {
		this->m_ip = ip;
	}

	void port(uint16_t port) {
		this->m_port = port;
	}

	void ipv6(bool ipv6) {
		this->m_ipv6 = ipv6;
	}

	// Following three functions are defined
	// in parent class conn_session_t, so they are virtual.
	virtual const std::string& ip() const;
	virtual uint16_t port() const;
	virtual bool ipv6() const;

	size_t pending_send_size() const {
		return this->m_send_buf.size() - this->m_send_offset;
	}

	uint32_t last_event_result() const {
		return this->m_last_event_result;
	}

	void last_event_result(uint32_t value) {
		this->m_last_event_result = value;
	}

	buf_t& recv_buf();
	buf_t& send_buf();

	// Receive data.
	err_t recv(size_t* new_recv_size, bool* peer_closed);

	// Send data.
	err_t send(size_t* new_send_size);

	// Get link node.
	//
	// acceptor_t saves conn_t in a doubly linked list.
	link_t<conn_t>* link_node() {
		return &this->m_link;
	}

private:
	// Remove default constructor, copy constructor and operator=().
	conn_t() = delete;
	conn_t(const conn_t&) = delete;
	conn_t& operator=(const conn_t&) = delete;

private:
	std::string m_ip;
	socket_t m_sd;
	uint16_t m_port;
	bool m_ipv6;
	link_t<conn_t> m_link;
	buf_t m_recv_buf;
	buf_t m_send_buf;
	size_t m_send_offset;
	uint32_t m_last_event_result;
};


}

