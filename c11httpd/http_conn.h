/**
 * HTTP connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/ctx.h"
#include "c11httpd/ctx_setter.h"
#include "c11httpd/fast_str.h"
#include "c11httpd/http_request.h"
#include "c11httpd/http_response.h"
#include <vector>


namespace c11httpd {


// HTTP connection.
class http_conn_t : public ctx_t, public ctx_setter_t {
public:
	http_conn_t() = default;
	virtual ~http_conn_t() = default;

	// Clear content.
	virtual void clear();

	// Get request object.
	http_request_t& request() {
		return this->m_request;
	}

	// Get response object.
	http_response_t& response() {
		return this->m_response;
	}

	// Get place holder values.
	std::vector<fast_str_t>& placeholders() {
		return this->m_placeholders;
	}

private:
	http_request_t m_request;
	http_response_t m_response;
	std::vector<fast_str_t> m_placeholders;
};


} // namespace c11httpd.

