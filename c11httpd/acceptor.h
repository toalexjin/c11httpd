/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn.h"
#include "c11httpd/conn_event.h"
#include "c11httpd/err.h"
#include "c11httpd/link.h"
#include "c11httpd/listen.h"
#include "c11httpd/waitable.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "listen.h"


namespace c11httpd {


// TCP acceptor.
//
// acceptor_t listens to a pile of specified TCP ports (ipv4 or ipv6),
// waits for incoming client connections. When a client connected,
// acceptor_t would create a conn_t object for the new connection.
// When the connection was disconnected, acceptor_t would put the
// conn_t object to a free list (for re-use) or destroy it.
class acceptor_t {
public:
	// "0.0.0.0"
	static const std::string ipv4_any;

	// "127.0.0.1"
	static const std::string ipv4_loopback;

	// "::"
	static const std::string ipv6_any;

	// "::1"
	static const std::string ipv6_loopback;

public:
	acceptor_t() {
		this->m_backlog = 10;
		this->m_max_events = 256;
		this->m_max_free_conn = 128;
	}

	virtual ~acceptor_t();
	virtual void close();

	// Listens to a port for both ipv4 & ipv6.
	err_t bind(uint16_t port);

	// Listens to a port.
	//
	// If "ip" is empty, the this function equals to acceptor_t::bind(port).
	err_t bind(const std::string& ip, uint16_t port);

	// Listens to an ipv4 port.
	//
	// If "ip" is empty, then acceptor_t::ipv4_any would be used.
	err_t bind_ipv4(const std::string& ip, uint16_t port);

	// Listens to an ipv6 port.
	//
	// If "ip" is empty, then acceptor_t::ipv6_any would be used.
	err_t bind_ipv6(const std::string& ip, uint16_t port);

	// Listens to a list of ports.
	err_t bind(std::initializer_list<std::pair<std::string, uint16_t>> list);

	// Get all listening ports.
	std::vector<std::pair<std::string, uint16_t>> binds() const;

	// Run TCP server service.
	//
	// "handler" is used to receive & handle client connection events.
	err_t run_tcp(conn_event_t* handler);

private:
	// Remove copy constructor, and operator=().
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;

private:
	err_t epoll_set_i(fd_t epoll, waitable_t* waitable, int op, uint32_t events);
	err_t epoll_del_i(fd_t epoll, waitable_t* waitable);
	void add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn);
	err_t loop_send_i(conn_event_t* handler, conn_t* conn);

private:
	std::vector<std::unique_ptr<listen_t>> m_listens;
	int m_backlog;
	int m_max_events;
	int m_max_free_conn;
};


} // namespace c11httpd.


