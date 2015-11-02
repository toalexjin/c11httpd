/**
 * Client connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/conn_session.h"
#include "c11httpd/ctx_setter.h"
#include "c11httpd/fd.h"
#include "c11httpd/link.h"
#include "c11httpd/listen.h"
#include "c11httpd/socket.h"
#include <aio.h>
#include <map>
#include <memory>
#include <string>


namespace c11httpd {


// Client connection.
//
// For each new client incoming connection, a conn_t object
// would be created. After the client connection was disconnected,
// the conn_t object might be re-used by acceptor_t for better performance.
class conn_t : public waitable_t, public conn_session_t, public ctx_setter_t {
private:
	class aio_node_t {
	public:
		aio_node_t() : m_link(uintptr_t(&this->m_link) - uintptr_t(this)){
			this->clear();
		}
		aio_node_t(const aio_node_t&) = default;
		aio_node_t& operator=(const aio_node_t&) = default;

		void clear() {
			bzero(&m_cb, sizeof(m_cb));
			m_id = 0;
			m_error = 0;
			m_ok_bytes = 0;
		}

		void to_pub(aio_t* pub) const {
			assert(pub != 0);

			pub->m_id = m_id;
			pub->m_error = m_error;
			pub->m_fd = m_cb.aio_fildes;
			pub->m_offset = m_cb.aio_offset;
			pub->m_buf = (char*) m_cb.aio_buf;
			pub->m_size = m_cb.aio_nbytes;
			pub->m_ok_bytes = m_ok_bytes;
		}

		link_t<aio_node_t> m_link;
		struct aiocb m_cb;
		int64_t m_id;
		err_t m_error;
		size_t m_ok_bytes;
	};

public:
	conn_t(const socket_t& sd, const std::string& ip, uint16_t port, bool ipv6)
		: waitable_t(waitable_t::type_conn),
		m_ip(ip), m_sd(sd), m_port(port), m_ipv6(ipv6),
		m_link(uintptr_t(&this->m_link) - uintptr_t(this)),
		m_aio_sequence(0) {

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

	// AIO operations.
	virtual err_t aio_read(fd_t fd, int64_t offset, char* buf, size_t size, int64_t* id);
	virtual err_t aio_write(fd_t fd, int64_t offset, const char* buf, size_t size, int64_t* id);
	virtual err_t aio_cancel(int64_t id);

	// Get completed AIO tasks and remove them from internal list.
	virtual void aio_completed(std::vector<aio_t>* completed);

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
	std::map<int64_t, std::unique_ptr<aio_node_t>> m_aio_running;
	std::map<int64_t, std::unique_ptr<aio_node_t>> m_aio_completed;
	int64_t m_aio_sequence;
};


}

