/**
 * Fast string.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <cstring>
#include <string>


namespace c11httpd {

// Fast string.
//
// The different between fast_str_t and std::string is that fast_str_t does not
// allocate memory and it always points to a string in an existing memory.
// As a result, its destructor does not free memory. With fast_str_t, we could
// avoid allocating many small memory while parsing a http header.
class fast_str_t {
public:
	const size_t npos = size_t(-1);

public:
	fast_str_t() : m_str(0), m_len(0) {
	}

	fast_str_t(const char* str)
		: fast_str_t(str, str == 0 ? 0 : std::strlen(str)) {
	}

	fast_str_t(const char* str, size_t len)
		: m_str(str), m_len(len) {
		assert(this->find(0) == npos);
	}

	fast_str_t(const std::string& str)
		: fast_str_t(str.c_str(), str.length()) {
	}

	fast_str_t(const fast_str_t&) = default;
	fast_str_t& operator=(const fast_str_t&) = default;

	size_t find(char ch) const {
		for (size_t i = 0; i < this->m_len; ++i) {
			if (this->m_str[i] == ch) {
				return i;
			}
		}

		return -1;
	}

private:
	const char* m_str;
	size_t m_len;
};

} // namespace c11httpd.


