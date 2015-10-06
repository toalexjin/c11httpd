/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn_base.h"
#include "c11httpd/err.h"
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
	acceptor_t() {
		this->m_backlog = 10;
		this->m_max_events = 64;
	}

	virtual ~acceptor_t();
	virtual void close();

	err_t bind(uint16_t port);
	err_t bind(const std::string& ip, uint16_t port);
	err_t bind_ipv4(const std::string& ip, uint16_t port);
	err_t bind_ipv6(const std::string& ip, uint16_t port);
	err_t bind(std::initializer_list<std::pair<std::string, uint16_t>> list);

	err_t run();

private:
	// Remove copy constructor, and operator=().
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;

private:
	static err_t epoll_add_i(fd_t epoll, conn_base_t* conn);
	static err_t epoll_del_i(fd_t epoll, conn_base_t* conn);

private:
	std::vector<std::unique_ptr<conn_base_t>> m_listens;
	int m_backlog;
	int m_max_events;
};


} // namespace c11httpd.


