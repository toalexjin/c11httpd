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
	this->m_vars.clear();
	this->m_hostname.clear();
	this->m_headers.clear();
	this->m_content = 0;
	this->m_content_length = 0;
	this->m_processed_bytes = 0;
	this->m_step = step_initial;
	this->m_split_items.clear();
}

const fast_str_t* http_request_t::var(const fast_str_t& name) const {
	assert(!name.empty());

	const http_var_t val(name, fast_str_t());
	const auto it = std::lower_bound(
			this->m_vars.cbegin(),
			this->m_vars.cend(),
			val);

	if (it != this->m_vars.cend() && !(val < (*it))) {
		return &((*it).value());
	} else {
		return 0;
	}
}

const fast_str_t* http_request_t::header(const fast_str_t& key) const {
	assert(!key.empty());

	const http_header_t val(key, fast_str_t());
	const auto it = std::lower_bound(
			this->m_headers.cbegin(),
			this->m_headers.cend(),
			val);

	if (it != this->m_headers.cend() && !(val < (*it))) {
		return &((*it).value());
	} else {
		return 0;
	}
}

http_request_t::parse_result_t http_request_t::continue_to_parse(
	const buf_t* recv_buf, size_t* bytes) {
	assert(recv_buf != 0);
	assert(recv_buf->size() >= this->m_processed_bytes);
	assert(bytes != 0);

	// Clear content.
	*bytes = 0;

	// "recv_buf" might have been re-allocated. If that's true,
	// all "fast_str_t" will become invalid, need to update.
	if (this->m_recv_buf != 0 && this->m_recv_buf != recv_buf->front()) {
		this->update_all_fast_str_i(this->m_recv_buf, recv_buf->front());
	}

	// Save recv_buf start position.
	this->m_recv_buf = recv_buf->front();

	fast_str_t line;
	next_line_t next_result;
	bool crlf;

	// First line: "GET /aa/bb HTTP/1.1"
	if (this->m_step < step_uri_done) {
		// Treat the entire package as a big message string.
		fast_str_t msg(recv_buf->front(), recv_buf->size());
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

		// Method.
		this->m_method = http_method_t::instance().to_integer(this->m_split_items[0]);
		if (this->m_method == http_method_t::unknown) {
			return parse_result_t::failed;
		}

		if (this->m_split_items[1].at(0) != '/') {
			return parse_result_t::failed;
		}

		fast_str_t uri, vars;
		const auto pos = this->m_split_items[1].find_first_of('?');
		if (pos == fast_str_t::npos) {
			uri = this->m_split_items[1];
		} else {
			uri = this->m_split_items[1].substr(0, pos);
			vars = this->m_split_items[1].substr(pos + 1);
		}

		// URI.
		if (!this->decode_i(uri, &this->m_uri)) {
			return parse_result_t::failed;
		}

		// HTTP Version.
		this->m_http_version = this->m_split_items[2];

		// Parse URI variables.
		if (!vars.empty()) {
			vars.split("&", &this->m_split_items);

			for (const auto& item : this->m_split_items) {
				fast_str_t name, value;

				const auto p2 = item.find_first_of('=');
				if (p2 == 0) {
					return parse_result_t::failed;
				}

				if (p2 == fast_str_t::npos) {
					if (!this->decode_i(item, &name)) {
						return parse_result_t::failed;
					}
				}
				else {
					if (!this->decode_i(item.substr(0, p2), &name)
						|| !this->decode_i(item.substr(p2 + 1), &value)) {
						return parse_result_t::failed;
					}
				}

				this->m_vars.push_back(http_var_t(name, value));
			}
		}

		// Sort URI variables.
		std::sort(this->m_vars.begin(), this->m_vars.end());

		this->m_processed_bytes = size_t(msg.c_str() - this->m_recv_buf);
		this->m_step = step_uri_done;
	}

	// Parse header attributes.
	if (this->m_step < step_header_done) {
		fast_str_t key, value;
		fast_str_t msg(recv_buf->front() + this->m_processed_bytes,
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
				std::sort(this->m_headers.begin(), this->m_headers.end());
				break;
			}

			if (!split_header_line_i(line, &key, &value)) {
				return parse_result_t::failed;
			}

			m_headers.push_back(http_header_t(key, value));
			this->m_processed_bytes = size_t(msg.c_str() - this->m_recv_buf);

			// Host name.
			if (this->m_hostname.empty() && key.cmpi("Host") == 0) {
				this->m_hostname = value.before(':');
			}
		}
	}

	if (this->m_step < step_content_done) {
		if (this->m_content_length == 0) {
			const fast_str_t* len_str = this->header("Content-Length");

			if (len_str != 0 && !len_str->empty()) {
				int32_t n;

				if (!len_str->to_number(&n)
					|| n < 0
					|| n > 10 * 1024 * 1024) {
					return parse_result_t::failed;
				}

				this->m_content_length = size_t(n);
			}
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

http_request_t::next_line_t http_request_t::next_line_i(
	fast_str_t* msg, fast_str_t* line, bool* crlf) {
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

	const auto pos = line.find_first_of(':');
	if (pos == 0 || pos == fast_str_t::npos || pos == line.length() - 1) {
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

void http_request_t::update_all_fast_str_i(const char* old_recv_buf, const char* new_recv_buf) {
	assert(old_recv_buf != 0);
	assert(new_recv_buf != 0);

	if (this->m_step >= step_uri_done) {
		update_single_fast_str_i(&this->m_uri, old_recv_buf, new_recv_buf);
		update_single_fast_str_i(&this->m_http_version, old_recv_buf, new_recv_buf);
	} else {
		return;
	}

	if (this->m_step >= step_header_done) {
		for (decltype(this->m_headers.size()) i = 0; i < this->m_headers.size(); ++i) {
			update_single_fast_str_i(&(this->m_headers[i].key()), old_recv_buf, new_recv_buf);
			update_single_fast_str_i(&(this->m_headers[i].value()), old_recv_buf, new_recv_buf);
		}
	} else {
		return;
	}
}

bool http_request_t::decode_i(const fast_str_t& encoded, fast_str_t* decoded) {
	char* ptr = (char*)encoded.c_str();
	size_t len = 0;
	size_t i = 0;

	while (i < encoded.length()) {
		if (ptr[i] == '%') {
			if ((i + 2) >= encoded.length()) {
				return false;
			}

			uint8_t value = 0;
			for (size_t k = 0; k < 2; ++k) {
				if (k > 0) {
					value <<= 4;
				}

				const char ch = ptr[i + 1 + k];
				if (ch >= '0' && ch <= '9') {
					value |= uint8_t(ch - '0');
				} else if (ch >= 'A' && ch <= 'F') {
					value |= uint8_t(ch - 'A' + 10);
				} else if (ch >= 'a' && ch <= 'f') {
					value |= uint8_t(ch - 'a' + 10);
				} else {
					return false;
				}
			}

			ptr[len] = char(value);
			++len;
			i += 3;
		} else {
			if (len != i) {
				ptr[len] = ptr[i];
			}

			++len;
			++i;
		}
	}

	decoded->set(ptr, len);
	return true;
}


} // namespace c11httpd.

