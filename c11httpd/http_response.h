/**
 * HTTP response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"


namespace c11httpd {


// HTTP response.
class http_response_t {
public:
	http_response_t() = default;
	virtual ~http_response_t() = default;

	// Clear content but do not free buffer.
	void clear() {
	}

	// Write response to the end of a buffer object.
	void append_to_buf(buf_t* send_buf);

private:
};


} // namespace c11httpd.

