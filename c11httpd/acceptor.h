/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn.h"
#include "c11httpd/conn_base.h"
#include "c11httpd/err.h"
#include "c11httpd/link.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>


namespace c11httpd {

/**
 * TCP acceptor.
 */
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

	err_t bind(uint16_t port);
	err_t bind(const std::string& ip, uint16_t port);
	err_t bind_ipv4(const std::string& ip, uint16_t port);
	err_t bind_ipv6(const std::string& ip, uint16_t port);
	err_t bind(std::initializer_list<std::pair<std::string, uint16_t>> list);

	std::vector<std::pair<std::string, uint16_t>> binds() const;
	err_t run();

private:
	// Remove copy constructor, and operator=().
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;

private:
	err_t epoll_add_i(fd_t epoll, conn_base_t* conn);
	err_t epoll_del_i(fd_t epoll, conn_base_t* conn);
	void add_free_conn_i(link_t<conn_t>* free_list, int* free_count, conn_t* conn);

private:
	std::vector<std::unique_ptr<conn_base_t>> m_listens;
	int m_backlog;
	int m_max_events;
	int m_max_free_conn;
};


} // namespace c11httpd.


