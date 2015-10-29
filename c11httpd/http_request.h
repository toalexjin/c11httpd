/**
 * HTTP request.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/fast_str.h"
#include "c11httpd/http_header.h"
#include "c11httpd/http_method.h"
#include <map>
#include <utility>
#include <algorithm>


namespace c11httpd {


// HTTP URI variable.
class http_var_t {
public:
	http_var_t() = default;
	http_var_t(const http_var_t&) = default;
	http_var_t& operator=(const http_var_t&) = default;

	http_var_t(const fast_str_t& name, const fast_str_t& value)
		: m_name(name), m_value(value) {
	}

	const fast_str_t& name() const {
		return this->m_name;
	}

	const fast_str_t& value() const {
		return this->m_value;
	}

	// Case sensitive.
	bool operator<(const http_var_t& another) const {
		return m_name.cmp(another.m_name) < 0;
	}

private:
	fast_str_t m_name;
	fast_str_t m_value;
};


template <typename T, typename Traits>
inline std::basic_ostream<T, Traits>& operator<<(
	std::basic_ostream<T, Traits>& ostream, const http_var_t& var) {
	return ostream << var.name() << "=" << var.value();
}


// HTTP request.
class http_request_t {
private:
	enum step_t {
		step_initial = 0,
		step_uri_done,
		step_header_done,
		step_content_done
	};

	enum class next_line_t {
		// A new line was gotten.
		ok,

		// There is no new-line character, need to receive more data.
		more,

		// A fatal error, e.g. there is a null terminal
		// character in the middle of the message.
		failed
	};

public:
	enum class parse_result_t {
		// OK.
		ok = 0,

		// Failed.
		failed = 1,

		// HTTP package is not completely received,
		// need to receive more data.
		more = 2
	};

public:
	http_request_t();
	virtual ~http_request_t() = default;

	// Clear content but do not free buffer.
	void clear();

	// Get method.
	int method() const {
		return this->m_method;
	}

	// Get URI.
	const fast_str_t& uri() const {
		return this->m_uri;
	}

	// Get HTTP version.
	const fast_str_t& http_version() const {
		return this->m_http_version;
	}

	// Get URI variable.
	//
	// If the variable does not exist, then NULL will be returned.
	const fast_str_t* var(const fast_str_t& name) const;

	// Get all URI variables.
	const std::vector<http_var_t>& vars() const {
		return this->m_vars;
	}

	// Get header value.
	//
	// If the specified header does not exist, then NULL will be returned.
	const fast_str_t* header(const fast_str_t& key) const;

	// Get host name (Similar to request header "Host:???", but does not have port number)
	const fast_str_t& hostname() const {
		return this->m_hostname;
	}

	// Get all headers.
	const std::vector<http_header_t>& headers() const {
		return this->m_headers;
	}

	// Get content.
	const char* content() const {
		return this->m_content;
	}

	// Get content size.
	size_t content_length() const {
		return this->m_content_length;
	}

	// Continue to parse request.
	//
	// A http request might be delivered in several TCP packets.
	// In order to enhance performance, "continue_to_parse" is able to
	// resume work from where it stopped last time.
	parse_result_t continue_to_parse(const buf_t* recv_buf, size_t* bytes);

private:
	static next_line_t next_line_i(fast_str_t* msg, fast_str_t* line, bool* crlf);
	static bool split_header_line_i(const fast_str_t& line, fast_str_t* key, fast_str_t* value);

	// Update fast_str_t's internal pointer.
	//
	// If recv_buf was re-allocated, we need to update
	// all parsed fast_str_t objects' internal pointers
	// before continue to parse the rest content of
	// a HTTP request.
	static void update_single_fast_str_i(fast_str_t* str,
		const char* old_recv_buf, const char* new_recv_buf) {
		assert(str != 0);
		assert(old_recv_buf != 0);
		assert(new_recv_buf != 0);

		if (str->c_str() != 0) {
			assert(str->c_str() > old_recv_buf);
			str->set(new_recv_buf + (str->c_str() - old_recv_buf), str->length());
		}
	}

	void update_all_fast_str_i(const char* old_recv_buf, const char* new_recv_buf);
	bool decode_i(const fast_str_t& encoded, fast_str_t* decoded);

private:
	// Remove copy constructor & operator=().
	http_request_t(const http_request_t&) = delete;
	http_request_t& operator=(const http_request_t&) = delete;

private:
	// Points to the starting location of the buffer.
	const char* m_recv_buf;
	int m_method;
	fast_str_t m_uri;
	fast_str_t m_http_version;
	std::vector<http_var_t> m_vars;

	// Similar to "Host:???", but does not have port number.
	fast_str_t m_hostname;

	// An ascending sorted header list (case insensitive).
	std::vector<http_header_t> m_headers;
	const char* m_content;
	size_t m_content_length;

	// Processed bytes of request package.
	size_t m_processed_bytes;
	step_t m_step;

	// Used as buffer.
	std::vector<fast_str_t> m_split_items;
};


} // namespace c11httpd.

