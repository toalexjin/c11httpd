/**
 * HTTP request.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/fast_str.h"
#include "c11httpd/http_method.h"
#include <map>
#include <utility>


namespace c11httpd {


// HTTP request.
class http_request_t {
public:
	typedef std::pair<fast_str_t, fast_str_t> header_value_t;

public:
	enum class parse_result_t {
		// OK.
		ok = 0,

		// Failed.
		failed = 1,

		// HTTP package is not completely received,
		// need to receive more data.
		more = 2
	};

public:
	http_request_t() {
		this->m_recv_buf = 0;
		this->m_method = http_method_t();
	}

	virtual ~http_request_t() = default;

	// Clear content but do not free buffer.
	void clear() {
		this->m_recv_buf = 0;
	}

	// Continue to parse request.
	parse_result_t continue_to_parse(const buf_t* recv_buf, size_t* bytes);

private:
	const char* m_recv_buf;
	http_method_t m_method;
	fast_str_t m_uri;
	fast_str_t m_version;

	// An ascending sorted header list.
	std::vector<header_value_t> m_headers;
};


} // namespace c11httpd.

