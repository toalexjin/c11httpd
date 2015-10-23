/**
 * HTTP request.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_request.h"


namespace c11httpd {


http_request_t::http_request_t() {
	this->m_recv_buf = 0;
	this->m_method = http_method_t::unknown;
	this->m_content = 0;
	this->m_content_length = 0;
	this->m_processed_bytes = 0;
	this->m_step = step_initial;
}

// Clear content but do not free buffer.
void http_request_t::clear() {
	this->m_recv_buf = 0;
	this->m_method = http_method_t::unknown;
	this->m_uri.clear();
	this->m_http_version.clear();
	this->m_host.clear();
	this->m_headers.clear();
	this->m_content = 0;
	this->m_content_length = 0;
	this->m_processed_bytes = 0;
	this->m_step = step_initial;
	this->m_split_items.clear();
}

const fast_str_t& http_request_t::header(const fast_str_t& key) const {
	const http_header_t val(key, fast_str_t());
	const auto it = std::lower_bound(
			this->m_headers.cbegin(),
			this->m_headers.cend(),
			val);

	if (it != this->m_headers.cend() && !(val < (*it))) {
		return (*it).value();
	} else {
		return fast_str_t::empty_string;
	}
}

http_request_t::parse_result_t http_request_t::continue_to_parse(
	const buf_t* recv_buf, size_t* bytes) {

	// Clear content.
	*bytes = 0;

	// "recv_buf" might have been re-allocated. If that's true,
	// all "fast_str_t" will become invalid, we need to clear.
	if (this->m_recv_buf != 0 && this->m_recv_buf != recv_buf->front()) {
		this->clear();
	}

	// Treat the entire request as a big message string.
	this->m_recv_buf = recv_buf->front();
	fast_str_t msg(recv_buf->front(), recv_buf->size());
	fast_str_t line;
	bool crlf;
	next_line_t next_result;
	fast_str_t key;
	fast_str_t value;

	// First line: "GET /aa/bb HTTP/1.1"
	if (this->m_step < step_uri_done) {
		next_result = next_line_i(&msg, &line, &crlf);
		if (next_result == next_line_t::failed) {
			return parse_result_t::failed;
		}

		if (next_result == next_line_t::more) {
			return parse_result_t::more;
		}

		if (line.split(" \t", &this->m_split_items) != 3) {
			return parse_result_t::failed;
		}

		this->m_method = http_method_t::instance().map(this->m_split_items[0]);
		if (this->m_method == http_method_t::unknown) {
			return parse_result_t::failed;
		}

		this->m_uri = this->m_split_items[1];
		this->m_http_version = this->m_split_items[2];

		this->m_processed_bytes = size_t(msg.c_str() - this->m_recv_buf);
		this->m_step = step_uri_done;
	}

	// Still in parsing header phrase.
	if (this->m_step < step_header_done) {
		msg.set(recv_buf->front() + this->m_processed_bytes,
			recv_buf->size() - this->m_processed_bytes);

		while (true) {
			next_result = next_line_i(&msg, &line, &crlf);
			if (next_result == next_line_t::failed) {
				return parse_result_t::failed;
			}

			if (next_result == next_line_t::more) {
				return parse_result_t::more;
			}

			// Parsing header is done.
			if (line.empty() && crlf) {
				this->m_processed_bytes = size_t(msg.c_str() - this->m_recv_buf);
				this->m_step = step_header_done;
				break;
			}

			if (!split_header_line_i(line, &key, &value)) {
				return parse_result_t::failed;
			}

			const auto key_ptr = http_header_t::all::instance().request_find(key);
			if (key_ptr == 0) {
				return parse_result_t::failed;
			}

			m_headers.push_back(http_header_t(*key_ptr, value));
			this->m_processed_bytes = size_t(msg.c_str() - this->m_recv_buf);
		}
	}

	if (this->m_step < step_content_done) {
		// Get "Host".
		if (m_host.empty()) {
			m_host = this->header("Host");
			if (m_host.empty()) {
				return parse_result_t::failed;
			}
		}

		if (this->m_content_length == 0) {
			int32_t n;

			if (!this->header("Content-Length").to_number(&n)
				|| n < 0
				|| n > 10 * 1024 * 1024) {
				return parse_result_t::failed;
			}

			this->m_content_length = size_t(n);
		}

		if (this->m_processed_bytes + this->m_content_length > recv_buf->size() ) {
			return parse_result_t::more;
		}

		m_content = this->m_recv_buf + this->m_processed_bytes;
		this->m_processed_bytes += this->m_content_length;
		this->m_step = step_content_done;
	}

	*bytes = this->m_processed_bytes;
	return parse_result_t::ok;
}

http_request_t::next_line_t http_request_t::next_line_i(fast_str_t* msg, fast_str_t* line, bool* crlf) {
	assert(msg != 0);
	assert(line != 0);
	assert(crlf != 0);

	for (size_t i = 0; i < msg->length(); ++i) {
		if (msg->at(i) == 0) {
			return next_line_t::failed;
		}

		if (msg->at(i) == '\n') {
			if (i > 0 && msg->at(i - 1) == '\r') {
				*crlf = true;
				line->set(msg->c_str(), i - 1);
			} else {
				*crlf = false;
				line->set(msg->c_str(), i);
			}

			*msg = msg->substr(i + 1);
			return next_line_t::ok;
		}
	}

	return next_line_t::more;
}

bool http_request_t::split_header_line_i(
	const fast_str_t& line, fast_str_t* key, fast_str_t* value) {
	assert(key != 0);
	assert(value != 0);

	auto pos = line.find_first_of(':');
	if (pos == 0 || pos == fast_str_t::npos) {
		return false;
	}

	key->set(line.c_str(), pos);
	value->set(line.c_str() + pos + 1, line.length() - pos - 1);

	key->trim();
	value->trim();

	if (key->empty() || value->empty()) {
		return false;
	}

	return true;
}


} // namespace c11httpd.

