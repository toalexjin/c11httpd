/**
 * HTTP method.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/fast_str.h"
#include <map>


namespace c11httpd {


// HTTP method.
class http_method_t {
public:
	enum type_t {
		unknown = 0,
		get,
		post,
		put,
		del,
		options,
		head
	};

public:
	static http_method_t& instance() {
		return st_instance;
	}

	// Map a method string to enumeration value.
	type_t map(const fast_str_t& str) const {
		const auto it = m_types.find(str);
		return it == m_types.end() ? unknown : (*it).second;
	}

private:
	http_method_t();
	http_method_t(const http_method_t&) = delete;
	http_method_t& operator=(const http_method_t&) = delete;

private:
	static http_method_t st_instance;

private:
	const std::map<fast_str_t, type_t, fast_str_less_nocase_t> m_types;
};


} // namespace c11httpd.

