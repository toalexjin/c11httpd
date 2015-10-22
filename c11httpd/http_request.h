/**
 * HTTP request.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"


namespace c11httpd {


// HTTP request.
class http_request_t {
public:
	enum class parse_result_t {
		// OK.
		ok = 0,

		// Failed.
		failed = 1,

		// Need more data to complete parsing.
		more = 2
	};

public:
	http_request_t() = default;
	virtual ~http_request_t() = default;

	// Clear content but do not free buffer.
	void clear() {
	}

	// Continue to parse request.
	parse_result_t continue_to_parse(const buf_t* recv_buf, size_t* bytes);

private:
};


} // namespace c11httpd.

