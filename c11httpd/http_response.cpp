/**
 * HTTP response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_response.h"
#include <cstring>


namespace c11httpd {


http_response_t& http_response_t::code(int code) {
	this->write_code_i(code);

	return *this;
}

http_response_t& http_response_t::operator<<(const http_header_t& header) {
	// Response header must be written before response content.
	assert(m_content_pos == 0);

	// "Content-Length" is written by http_response_t automatically,
	if (header.key().cmpi("Content-Length") == 0) {
		assert(false);
		return *this;
	}

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

void http_response_t::write_code_i(int code, const char* http_version) {
	// Range [100,599]
	assert(code >= 100 && code <= 599);

	if (this->m_header_pos == 0) {
		*m_send_buf << http_version << " ";
		this->m_code_pos = m_send_buf->size();
		*m_send_buf << code << " OK\r\n";
		this->m_header_pos = m_send_buf->size();
	} else {
		// If HTTP status code is no change, then return immediately.
		if (this->m_code == code) {
			return;
		}

		constexpr int buf_len = 7;

		if (code >= 400 && code <= 599) {
			std::snprintf(m_send_buf->front() + this->m_code_pos, buf_len, "%d ER", code);
		} else {
			std::snprintf(m_send_buf->front() + this->m_code_pos, buf_len, "%d OK", code);
		}

		m_send_buf->front()[this->m_code_pos + buf_len - 1] = '\r';
	}

	this->m_code = code;
}

void http_response_t::complete_header_i() {
	assert(m_header_pos != 0);
	assert(m_content_pos == 0);

	// "Server:c11httpd"
	*m_send_buf << "Server: c11httpd\r\n";

	// "Content-Length:       0"
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

