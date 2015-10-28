/**
 * HTTP response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/fast_str.h"
#include "c11httpd/http_header.h"
#include "c11httpd/http_status.h"
#include <string>


namespace c11httpd {


// HTTP response.
class http_response_t {
public:
	http_response_t() {
		this->clear();
	}

	virtual ~http_response_t() = default;

	// Clear content but do not free buffer.
	void clear() {
		this->m_send_buf = 0;
		this->m_code = http_status_t::ok;
		this->m_header_pos = 0;
		this->m_content_len_pos = 0;
		this->m_content_pos = 0;
	}

	void attach(buf_t* send_buf) {
		this->clear();
		this->m_send_buf = send_buf;
	}

	void detach() {
		this->complete_content_i();
		this->clear();
	}

	// Write HTTP status code.
	http_response_t& code(int code);

	// Write header.
	http_response_t& operator<<(const http_header_t& header);

	// Write content.
	http_response_t& write(const void* data, size_t size);
	http_response_t& operator<<(const char* str);
	http_response_t& operator<<(const std::string& str);
	http_response_t& operator<<(const fast_str_t& str);

private:
	http_response_t(const http_response_t&) = delete;
	http_response_t& operator=(const http_response_t&) = delete;

	void write_code_i(int code = http_status_t::ok);
	void complete_header_i();
	void complete_content_i();

private:
	buf_t* m_send_buf;

	// HTTP status code.
	int m_code;

	// Where response header is located.
	size_t m_header_pos;

	// Where content length is located (totally 8 characters available).
	size_t m_content_len_pos;

	// Where response content is located.
	size_t m_content_pos;
};


} // namespace c11httpd.

