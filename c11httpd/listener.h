/**
 * TCP listener.
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
 * TCP listener.
 */
class listener_t {
public:
	listener_t() = default;
	virtual ~listener_t();
	void destroy();

	err_t bind(uint16_t port);
	err_t bind(const char* ip, uint16_t port);
	err_t bind_ipv4(const char* ip, uint16_t port);
	err_t bind_ipv6(const char* ip, uint16_t port);
	err_t bind(std::initializer_list<std::pair<std::string, uint16_t>> list);

private:
	listener_t(const listener_t&) = delete;
	listener_t(listener_t&&) = delete;
	listener_t& operator=(const listener_t&) = delete;
	listener_t& operator=(listener_t&&) = delete;

private:
	void resize_i(size_t new_size);

private:
	std::vector<int> m_binds;
};


} // namespace c11httpd.


