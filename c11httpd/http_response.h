/**
 * HTTP response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/config.h"
#include "c11httpd/fast_str.h"
#include "c11httpd/http_header.h"
#include "c11httpd/http_request.h"
#include "c11httpd/http_status.h"
#include "c11httpd/rest_result.h"
#include <string>
#include <set>
#include <vector>


namespace c11httpd {


// HTTP response.
//
// http_response_t enables you to write response header & content
// with serialization (operator<<) easily. The only limitation
// is that you must write response header before writing response content.
// However, response status code is allowed to update at any time.
class http_response_t {
public:
	http_response_t() {
		this->clear();
	}

	~http_response_t() = default;

	// Clear content but do not free buffer.
	void clear() {
		this->m_config = 0;
		this->m_request = 0;
		this->m_default_response_content_type = 0;
		this->m_send_buf = 0;
		this->m_code = http_status_t::ok;
		this->m_code_pos = 0;
		this->m_header_pos = 0;
		this->m_content_len_pos = 0;
		this->m_content_pos = 0;
		this->m_split_items.clear();
		this->m_content_type_done = false;
	}

	void attach(const config_t* cfg,
		const http_request_t* request,
		const std::string* default_response_content_type,
		buf_t* send_buf) {
		this->clear();

		this->m_config = cfg;
		this->m_request = request;
		this->m_default_response_content_type = default_response_content_type;
		this->m_send_buf = send_buf;
	}

	void detach(rest_result_t result) {
		if (result != rest_result_t::abandon) {
			this->complete_content_i();
		}

		this->clear();
	}

	// Get HTTP status code.
	int code() const {
		return this->m_code;
	}

	// Update response status code.
	//
	// Unlike response header & content, response status code
	// is permitted to update at any time.
	http_response_t& code(int code);

	// Write response header.
	//
	// You must write response header before writing response content.
	// <BR>
	//
	// Note that you do not need to write header "Content-Length"
	// because it's handled by http_response_t automatically.
	http_response_t& operator<<(const http_header_t& header);

	// Write response content.
	//
	// You must write response header before writing response content.
	http_response_t& write(const void* data, size_t size);
	http_response_t& operator<<(const char* str);
	http_response_t& operator<<(const std::string& str);
	http_response_t& operator<<(const fast_str_t& str);

private:
	http_response_t(const http_response_t&) = delete;
	http_response_t& operator=(const http_response_t&) = delete;

	void write_code_i(int code = http_status_t::ok,
		const fast_str_t& http_version = http_header_t::HTTP_VERSION_1_1);

	void complete_header_i();
	void complete_content_i();

private:
	// Some headers are protected, not allowed to update by caller.
	static const std::set<fast_str_t, fast_str_less_nocase_t> st_protected_headers;

private:
	const config_t* m_config;
	const http_request_t* m_request;
	const std::string* m_default_response_content_type;
	buf_t* m_send_buf;

	// HTTP status code.
	int m_code;

	// Where response status code is located.
	size_t m_code_pos;

	// Where response header is located.
	size_t m_header_pos;

	// Where content length is located (totally 8 characters available to write).
	size_t m_content_len_pos;

	// Where response content is located.
	size_t m_content_pos;

	// Used as buffer.
	std::vector<fast_str_t> m_split_items;

	// If "Content-Type:???" has been written.
	bool m_content_type_done;
};


} // namespace c11httpd.

