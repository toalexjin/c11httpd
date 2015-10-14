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
#include "c11httpd/worker_pool.h"
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
#include <set>


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

	// Get number of worker processes.
	int worker_processes() const {
		return this->m_worker_processes;
	}

	// Set number of worker processes.
	//
	// -# If the number is zero, then the main process does everything,
	//    including receiving incoming client requests.
	// -# If the number is greater than zero, than the main process
	//    would become a pure management process, will NOT receive
	//    incoming client requests and will restart worker processes if they died.
	void worker_processes(int worker_processes) {
		assert(worker_processes >= 1);
		this->m_worker_processes = worker_processes;
	}

	// Return true if it's not worker process.
	bool main_process() const {
		return this->m_worker_pool.main_process();
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
	// or triggered by Linux signal.
	err_t stop();

	// Triggered when a Linux signal is received.
	virtual void on_signalled(int signum);

private:
	// Remove copy constructor, and operator=().
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;

private:
	err_t epoll_set_i(fd_t epoll, socket_t sock, waitable_t* waitable, int op, uint32_t events);
	err_t epoll_del_i(fd_t epoll, socket_t sock);
	void add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn);
	err_t loop_send_i(conn_event_t* handler, conn_t* conn);
	err_t create_signal_sock_i();
	void close_signal_sock_i();
	err_t recv_signal_sock_i(fd_t* epoll, bool* exit);
	err_t send_signal_sock_i(int signum);

private:
	std::vector<std::unique_ptr<listen_t>> m_listens;
	worker_pool_t m_worker_pool;
	std::recursive_mutex m_signal_sock_mutex;
	socket_t m_signal_sock[2];
	int m_worker_processes;
	int m_backlog;
	int m_max_events;
	int m_max_free_conn;
};


} // namespace c11httpd.


