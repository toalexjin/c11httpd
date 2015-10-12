/**
 * Client connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/conn_base.h"
#include "c11httpd/conn_session.h"
#include "c11httpd/link.h"
#include "c11httpd/socket.h"
#include <string>


namespace c11httpd {


// Client connection.
//
// For each new client incoming connection, a conn_t object
// would be created. After the client connection was disconnected,
// the conn_t object might be re-used by acceptor_t for better performance.
class conn_t : public conn_base_t, public conn_session_t {
public:
	conn_t(const socket_t& sd, const std::string& ip, uint16_t port, bool ipv6)
		: conn_base_t(sd, ip, port, false, ipv6),
		m_link(uintptr_t(&this->m_link) - uintptr_t(this)) {

		assert(this == this->m_link.get());
		this->m_send_offset = 0;
		this->m_last_event_result = 0;
	}

	virtual ~conn_t();

	// Close internal handles, reset internal variables, but do not free memory.
	//
	// acceptor_t would call conn_t::close() and then
	// put the object to a free conn_t list for re-use.
	virtual void close();

	void ip(const std::string& ip) {
		conn_base_t::ip(ip);
	}

	void port(uint16_t port) {
		conn_base_t::port(port);
	}

	void ipv6(bool ipv6) {
		conn_base_t::ipv6(ipv6);
	}

	virtual const std::string& ip() const;
	virtual uint16_t port() const;
	virtual bool ipv6() const;

	size_t send_pending_size() const {
		return this->m_send_buf.size() - this->m_send_offset;
	}

	uint32_t last_event_result() const {
		return this->m_last_event_result;
	}

	void last_event_result(uint32_t value) {
		this->m_last_event_result = value;
	}

	buf_t* recv_buf();
	buf_t* send_buf();

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
	link_t<conn_t> m_link;
	buf_t m_recv_buf;
	buf_t m_send_buf;
	size_t m_send_offset;
	uint32_t m_last_event_result;
};


}

