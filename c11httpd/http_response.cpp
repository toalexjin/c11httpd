/**
 * HTTP response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_response.h"
#include "c11httpd/utility.h"
#include <cstring>


namespace c11httpd {


const std::set<fast_str_t, fast_str_less_nocase_t> http_response_t::st_protected_headers = {
	http_header_t::Connection,
	http_header_t::Content_Length,
	http_header_t::Date,
	http_header_t::Server
};


http_response_t& http_response_t::code(int code) {
	this->write_code_i(code);

	return *this;
}

http_response_t& http_response_t::operator<<(const http_header_t& header) {
	// Response header must be written before response content.
	assert(m_content_pos == 0);

	// Some headers are not allowed to update.
	if (st_protected_headers.find(header.key()) != st_protected_headers.end()) {
		assert(false);
		return *this;
	}

	// If first line is not written, then write it.
	if (this->m_header_pos == 0) {
		this->write_code_i();
	}

	// Mark Content-Type:??? as written.
	if (!this->m_content_type_done) {
		if (header.key().cmpi(http_header_t::Content_Type) == 0) {
			this->m_content_type_done = true;
		}
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

void http_response_t::write_code_i(int code, const fast_str_t& http_version) {
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

	// "Connection: keep-alive"
	if (this->m_config->enabled(config_t::keep_alive)) {
		const fast_str_t* value = this->m_request->header(http_header_t::Connection);
		if (value != 0) {
			value->split(" \t,", &this->m_split_items);

			for (const auto& item : this->m_split_items) {
				if (item.cmpi(http_header_t::Keep_Alive) == 0) {
					*m_send_buf << http_header_t::Connection << ": "
								<< http_header_t::Keep_Alive << "\r\n";
					break;
				}
			}
		}
	}

	// "Date: ???"
	if (this->m_config->enabled(config_t::response_date)) {
		char str[utility_t::response_date_len];

		utility_t::response_date(str);
		*m_send_buf << http_header_t::Date << ": " << str << "\r\n";
	}

	// "Server: c11httpd"
	*m_send_buf << http_header_t::Server << ": c11httpd\r\n";

	// "Content-Type: ???"
	if (!this->m_content_type_done
		&& this->m_default_response_content_type != 0
		&& !this->m_default_response_content_type->empty()) {
		*m_send_buf << http_header_t::Content_Type << ": "
				<< *m_default_response_content_type << "\r\n";
	}

	// "Content-Length:       0"
	*m_send_buf << http_header_t::Content_Length << ":";
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

