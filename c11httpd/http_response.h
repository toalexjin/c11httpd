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
	http_response_t() : m_send_buf(0) {
	}

	virtual ~http_response_t() = default;

	// Clear content but do not free buffer.
	void clear() {
		this->m_send_buf = 0;
	}

	void attach(buf_t* send_buf) {
		this->clear();
		this->m_send_buf = send_buf;
	}

private:
	buf_t* m_send_buf;
};


} // namespace c11httpd.

