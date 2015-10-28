/**
 * HTTP response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_response.h"
#include <cstring>


namespace c11httpd {


http_response_t& http_response_t::code(int code) {
	// HTTP status code must be written before header & content.
	assert(this->m_header_pos == 0);

	this->write_code_i(code);
	return *this;
}

http_response_t& http_response_t::operator<<(const http_header_t& header) {
	// Response header must be written before response content.
	assert(m_content_pos == 0);

	// "Content-Length" is written automatically,
	// and caller is not permitted to write this field directly.
	assert(header.key().cmpi("Content-Length") != 0);

	// If first line is not written, then write it.
	if (this->m_header_pos == 0) {
		this->write_code_i();
	}

	*m_send_buf << header.key() << ": " << header.value() << "\r\n";

	return *this;
}

http_response_t& http_response_t::write(const void* data, size_t size) {
	assert(data != 0 || size == 0);

	// If first line is not written, then write it.
	if (this->m_header_pos == 0) {
		this->write_code_i();
	}

	if (this->m_content_pos == 0) {
		this->complete_header_i();
	}

	this->m_send_buf->push_back(data, size);
	return *this;
}

http_response_t& http_response_t::operator<<(const char* str) {
	if (str != 0) {
		this->write(str, std::strlen(str));
	}

	return *this;
}

http_response_t& http_response_t::operator<<(const std::string& str) {
	return this->write(str.c_str(), str.length());
}

http_response_t& http_response_t::operator<<(const fast_str_t& str) {
	return this->write(str.c_str(), str.length());
}

void http_response_t::write_code_i(int code) {
	*m_send_buf << "HTTP/1.1 " << code << " OK\r\n";
	this->m_code = code;
	this->m_header_pos = m_send_buf->size();
}

void http_response_t::complete_header_i() {
	assert(m_header_pos != 0);
	assert(m_content_pos == 0);

	*m_send_buf << "Content-Length:";
	m_content_len_pos = m_send_buf->size();
	*m_send_buf << "       0\r\n";
	*m_send_buf << "\r\n";
	m_content_pos = m_send_buf->size();
}

void http_response_t::complete_content_i() {
	// If first line is not written, then write it.
	if (this->m_header_pos == 0) {
		this->write_code_i();
	}

	if (this->m_content_pos == 0) {
		this->complete_header_i();
	}

	const int content_len = int(this->m_send_buf->size() - this->m_content_pos);
	if (content_len > 0) {
		char buf[32];
		const int str_len = std::snprintf(buf, sizeof(buf), "%d", content_len);
		if (str_len > 0 && str_len <= 8) {
			char* ptr = this->m_send_buf->front() + this->m_content_len_pos + (8 - str_len);
			for (int i = 0; i < str_len; ++i) {
				ptr[i] = buf[i];
			}
		}
	}
}


} // namespace c11httpd.

