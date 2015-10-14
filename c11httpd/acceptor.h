/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn.h"
#include "c11httpd/conn_event.h"
#include "c11httpd/conn_event_adapter.h"
#include "c11httpd/err.h"
#include "c11httpd/link.h"
#include "c11httpd/listen.h"
#include "c11httpd/signal_event.h"
#include "c11httpd/socket.h"
#include "c11httpd/waitable.h"
#include <initializer_list>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <functional>


namespace c11httpd {


// TCP acceptor.
//
// acceptor_t listens to a pile of specified TCP ports (ipv4 or ipv6),
// waits for incoming client connections. When a client connected,
// acceptor_t would create a conn_t object for the new connection.
// When the connection was disconnected, acceptor_t would put the
// conn_t object to a free list (for re-use) or destroy it.
class acceptor_t : public waitable_t, public signal_event_t {
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
	acceptor_t();
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

	// Which Linux signals could stop the service.
	const std::vector<int>& stop_signals() {
		return this->m_stop_signals;
	}

	// Which Linux signals could stop the service.
	void stop_signals(const std::vector<int>& stop_signals) {
		this->m_stop_signals = stop_signals;
	}

	// Run TCP server service.
	//
	// "handler" is used to receive & handle client connection events.
	err_t run_tcp(conn_event_t* handler);

	// Run TCP server service.
	err_t run_tcp(const conn_event_adapter_t::on_received_t& recv);

	// Stop the service.
	//
	// Note that this function could be called from another thread
	// triggered by Linux signal.
	err_t stop();

	// Triggered when a Linux signal is received.
	virtual void on_signal(int signum);

private:
	// Remove copy constructor, and operator=().
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;

private:
	err_t stop_i(int signum);
	err_t epoll_set_i(fd_t epoll, socket_t sock, waitable_t* waitable, int op, uint32_t events);
	err_t epoll_del_i(fd_t epoll, socket_t sock);
	void add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn);
	err_t loop_send_i(conn_event_t* handler, conn_t* conn);
	err_t create_stop_sock_i();
	void close_stop_sock_i();

private:
	std::vector<std::unique_ptr<listen_t>> m_listens;
	std::vector<int> m_stop_signals;
	std::recursive_mutex m_stop_sock_mutex;
	socket_t m_stop_sock[2];
	int m_backlog;
	int m_max_events;
	int m_max_free_conn;
};


} // namespace c11httpd.


