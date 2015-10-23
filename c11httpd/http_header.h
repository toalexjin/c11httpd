/**
 * HTTP header.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/fast_str.h"


namespace c11httpd {


// HTTP header.
//
// <B>Example:</B>
// @code
// http_response response;
//
// response << http_header_t("Content-Type", "application/json;charset=UTF-8")
//          << http_header_t("Content-Encoding", "gzip");
// @endcode
class http_header_t {
public:
	// All header keys.
	//
	// This single instance defines all possible HTTP header keys.
	class all {
	public:
		static const all& instance() {
			return st_instance;
		}

		const std::vector<fast_str_t> request_all() const {
			return this->m_request;
		}

		const std::vector<fast_str_t> response_all() const {
			return this->m_response;
		}

		const fast_str_t* request_find(const fast_str_t& key) const;
		const fast_str_t* response_find(const fast_str_t& key) const;

	private:
		all();
		all(const all& another) = delete;
		all& operator=(const all& another) = delete;

	private:
		static all st_instance;

	private:
		std::vector<fast_str_t> m_request;
		std::vector<fast_str_t> m_response;
		fast_str_less_nocase_t m_less;
	};


public:
	http_header_t() = default;
	http_header_t(const fast_str_t& key, const fast_str_t& value)
		: m_key(key), m_value(value) {
	}

	const fast_str_t& key() const {
		return this->m_key;
	}

	void key(const fast_str_t& key) {
		this->m_key = key;
	}

	const fast_str_t& value() const {
		return this->m_value;
	}

	void value(const fast_str_t& value) {
		this->m_value = value;
	}

	bool operator<(const http_header_t& another) const {
		return m_key.cmpi(another.m_key) < 0;
	}

private:
	fast_str_t m_key;
	fast_str_t m_value;
};


} // namespace c11httpd.

