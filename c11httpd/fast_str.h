/**
 * Fast string.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <cstring>
#include <string>
#include <vector>


namespace c11httpd {


// Fast string.
//
// The different between fast_str_t and std::string is that fast_str_t does not
// allocate memory and it always points to a string in an existing memory.
// As a result, its destructor does not free memory. With fast_str_t, we could
// avoid allocating many small memory while parsing a http header.
class fast_str_t {
public:
	// Not-found.
	static const size_t npos = size_t(-1);

	// An empty string.
	static const fast_str_t empty_string;

public:
	fast_str_t() : m_str(0), m_len(0) {
	}

	fast_str_t(const char* str)
		: fast_str_t(str, str == 0 ? 0 : std::strlen(str)) {
	}

	fast_str_t(const char* str, size_t len)
		: m_str(str), m_len(len) {
	}

	fast_str_t(const std::string& str)
		: fast_str_t(str.c_str(), str.length()) {
	}

	fast_str_t(const fast_str_t&) = default;
	fast_str_t& operator=(const fast_str_t&) = default;

	void clear() {
		this->m_str = 0;
		this->m_len = 0;
	}

	bool empty() const {
		return this->m_len == 0;
	}

	size_t length() const {
		return this->m_len;
	}

	const char* c_str() const {
		return m_str;
	}

	char at(size_t index) const {
		assert(index < this->m_len);
		return this->m_str[index];
	}

	char operator[](size_t index) const {
		return this->at(index);
	}

	void set(const char* str, size_t len) {
		this->m_str = str;
		this->m_len = len;
	}

	fast_str_t substr(size_t pos = 0, size_t len = npos) const {
		assert(len == npos || pos + len <= m_len);
		return fast_str_t(m_str + pos, len == npos ? (m_len - pos) : len);
	}

	bool operator==(const fast_str_t& another) const {
		return this->cmp(another) == 0;
	}

	bool operator!=(const fast_str_t& another) const {
		return this->cmp(another) != 0;
	}

	int cmp(const fast_str_t& another) const;
	int cmpi(const fast_str_t& another) const;

	// Split string.
	//
	// @return Number of items.
	size_t split(const char* delims, std::vector<fast_str_t>* items) const;

	size_t find(char ch) const;
	size_t find_first_of(char ch, size_t pos = 0) const;
	size_t find_first_of(const char* delims, size_t pos = 0) const;
	size_t find_first_not_of(char ch, size_t pos = 0) const;
	size_t find_first_not_of(const char* delims, size_t pos = 0) const;

	// Trim whitespaces, tab, etc.
	//
	// @return Number of removed characters.
	size_t trim_left(const char* delims = " \t");
	size_t trim_right(const char* delims = " \t");

	// Trim both sides.
	size_t trim(const char* delims = " \t");

	bool to_number(int32_t* value) const;
	bool to_number(uint32_t* value) const;

	int32_t to_i32(int32_t default_value = 0) const;
	uint32_t to_u32(uint32_t default_value = 0) const;

private:
	const char* m_str;
	size_t m_len;
};


// No-case less operator for fast_str_t.
class fast_str_less_nocase_t {
public:
	bool operator()(const fast_str_t& first, const fast_str_t& second) const {
		return first.cmpi(second) < 0;
	}
};


} // namespace c11httpd.


