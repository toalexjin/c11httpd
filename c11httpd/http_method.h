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
	enum {
		unknown = 0,
		any = 1,
		get = 2,
		post = 3,
		put = 4,
		del = 5,
		options = 6,
		head = 7
	};

public:
	static http_method_t& instance() {
		return st_instance;
	}

	// Map string to integer value.
	int to_integer(const fast_str_t& str) const;

	// Map integer value to string.
	const fast_str_t& to_str(int method);

private:
	http_method_t();
	http_method_t(const http_method_t&) = delete;
	http_method_t& operator=(const http_method_t&) = delete;

private:
	static http_method_t st_instance;

private:
	const std::map<fast_str_t, int, fast_str_less_nocase_t> m_str_index;
	fast_str_t m_integer_index[head + 1];
};


} // namespace c11httpd.

