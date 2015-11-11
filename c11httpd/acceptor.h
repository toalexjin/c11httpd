/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/config.h"
#include "c11httpd/conn.h"
#include "c11httpd/conn_event.h"
#include "c11httpd/conn_event_adapter.h"
#include "c11httpd/err.h"
#include "c11httpd/link.h"
#include "c11httpd/listen.h"
#include "c11httpd/waitable.h"
#include "c11httpd/worker_pool.h"
#include "c11httpd/rest_ctrl.h"
#include "c11httpd/socket.h"
#include <functional>
#include <initializer_list>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>


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
	explicit acceptor_t(const config_t& cfg = config_t());
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

	// Return true if it's not worker process.
	bool main_process() const {
		return this->m_worker_pool.main_process();
	}

	// Run TCP server service.
	//
	// If Linux signal SIGINT or SIGTERM is received, this function will return.
	// "handler" is used to receive & handle client connection events.
	err_t run_tcp(conn_event_t* handler);

	// Run TCP server service.
	//
	// If Linux signal SIGINT or SIGTERM is received, this function will return.
	err_t run_tcp(const conn_event_adapter_t::on_received_t& recv);

	// Run RESTFul service.
	//
	// If Linux signal SIGINT or SIGTERM is received, this function will return.
	err_t run_http(rest_ctrl_t* controller);

	// Run RESTFul service.
	//
	// If Linux signal SIGINT or SIGTERM is received, this function will return.
	err_t run_http(const std::vector<rest_ctrl_t*>& controllers);

	// Get configuration.
	const config_t& config() const {
		return this->m_config;
	}

	// Get configuration.
	config_t& config() {
		return this->m_config;
	}

	// Set configuration.
	void config(const config_t& cfg) {
		this->m_config = cfg;
	}

	// Stop the service.
	//
	// Note that this function could be called from another thread
	// or triggered by Linux signal.
	err_t stop();

private:
	// Remove copy constructor, and operator=().
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;

private:
	// Add/update epoll item.
	err_t epoll_set_i(fd_t epoll, socket_t sock,
		const waitable_t* waitable, int op, uint32_t events);

	// Delete epoll item.
	err_t epoll_del_i(fd_t epoll, socket_t sock);

	// Add a free connection object.
	void add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn);

	// Send data until send_buf is full.
	err_t loop_send_i(conn_event_t* handler, conn_t* conn);

	// Linux signal received.
	err_t on_signalled_i(fd_t epoll, fd_t signal_fd, bool* exit, std::set<conn_t*>* aio_conns);

	// Return number of terminated worker processes.
	int on_worker_terminated_i();

	// Restart terminated worker processes.
	err_t restart_worker_i(fd_t epoll, int dead_workers);

	// Garbage-collect a connection.
	void gc_conn_i(conn_event_t* handler,
		fd_t epoll, conn_t* conn, bool new_conn, int* used_count,
		link_t<conn_t>* free_list, int* free_count);

private:
	std::vector<std::unique_ptr<listen_t>> m_listens;
	worker_pool_t m_worker_pool;
	config_t m_config;
	buf_t m_signal_buf;
};


} // namespace c11httpd.


