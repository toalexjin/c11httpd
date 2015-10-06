/**
 * TCP acceptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include <string>
#include <vector>
#include <initializer_list>
#include <utility>


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
	void destroy();

	err_t bind(uint16_t port);
	err_t bind(const char* ip, uint16_t port);
	err_t bind_ipv4(const char* ip, uint16_t port);
	err_t bind_ipv6(const char* ip, uint16_t port);
	err_t bind(std::initializer_list<std::pair<std::string, uint16_t>> list);

	err_t accept();

private:
	acceptor_t(const acceptor_t&) = delete;
	acceptor_t(acceptor_t&&) = delete;
	acceptor_t& operator=(const acceptor_t&) = delete;
	acceptor_t& operator=(acceptor_t&&) = delete;

private:
	void resize_i(size_t new_size);
	static err_t epoll_add_server_i(int epoll, int new_fd);

private:
	std::vector<int> m_binds;
	int m_backlog;
	int m_max_events;
};


} // namespace c11httpd.


