/**
 * HTTP header fields.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/fast_str.h"


namespace c11httpd {


// HTTP header fields.
//
// This class includes all request & response header fields.
class http_header_fields_t {
public:
	static const http_header_fields_t& instance() {
		return st_instance;
	}

	const std::vector<fast_str_t> request_fields() const {
		return this->m_request_fields;
	}

	const std::vector<fast_str_t> response_fields() const {
		return this->m_response_fields;
	}

private:
	http_header_fields_t();

	static http_header_fields_t st_instance;

private:
	std::vector<fast_str_t> m_request_fields;
	std::vector<fast_str_t> m_response_fields;
};


} // namespace c11httpd.

