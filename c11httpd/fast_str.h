/**
 * Fast string.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <cstring>
#include <string>
#include <algorithm>


namespace c11httpd {


// Fast string.
//
// The different between fast_str_t and std::string is that fast_str_t does not
// allocate memory and it always points to a string in an existing memory.
// As a result, its destructor does not free memory. With fast_str_t, we could
// avoid allocating many small memory while parsing a http header.
class fast_str_t {
public:
	static const size_t npos = size_t(-1);

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

	char at(size_t index) const {
		assert(index < this->m_len);
		return this->m_str[index];
	}

	char operator[](size_t index) const {
		return this->at(index);
	}

	fast_str_t substr(size_t pos = 0, size_t len = npos) const {
		assert(len == npos || pos + len <= m_len);
		return fast_str_t(m_str + pos, len == npos ? (m_len - pos) : len);
	}

	size_t find(char ch) const {
		for (size_t i = 0; i < this->m_len; ++i) {
			if (this->m_str[i] == ch) {
				return i;
			}
		}

		return npos;
	}

	size_t find_first_of(char ch, size_t pos = 0) const {
		for (size_t i = pos; i < this->m_len; ++i) {
			if (ch == this->m_str[i]) {
				return i;
			}
		}

		return npos;
	}

	size_t find_first_of(const char* s, size_t pos = 0) const {
		for (size_t i = pos; i < this->m_len; ++i) {
			for (const char* ptr = s; *ptr != 0; ++ptr) {
				if (*ptr == this->m_str[i]) {
					return i;
				}
			}
		}

		return npos;
	}

	size_t find_first_not_of(char ch, size_t pos = 0) const {
		for (size_t i = pos; i < this->m_len; ++i) {
			if (ch != this->m_str[i]) {
				return i;
			}
		}

		return npos;
	}

	size_t find_first_not_of(const char* s, size_t pos = 0) const {
		for (size_t i = pos; i < this->m_len; ++i) {
			const char* ptr = s;

			while (*ptr != 0 && *ptr != this->m_str[i]) {
				++ptr;
			}

			if (*ptr == 0) {
				return i;
			}
		}

		return npos;
	}

	int cmp(const fast_str_t& another) const {
		const size_t min_len = std::min(this->m_len, another.m_len);

		for (size_t i = 0; i < min_len; ++i) {
			if (uint8_t(this->m_str[i]) < uint8_t(another.m_str[i])) {
				return -1;
			} else if (uint8_t(this->m_str[i]) > uint8_t(another.m_str[i])) {
				return 1;
			}
		}

		if (this->m_len < another.m_len) {
			return -1;
		} else if (this->m_len > another.m_len) {
			return 1;
		}

		return 0;
	}

	int cmpi(const fast_str_t& another) const {
		const size_t min_len = std::min(this->m_len, another.m_len);

		for (size_t i = 0; i < min_len; ++i) {
			const uint8_t first = (this->m_str[i] >= 'A' && this->m_str[i] <= 'Z')
					? uint8_t(this->m_str[i] - 'A' + 'a') : uint8_t(this->m_str[i]);

			const uint8_t second = (another.m_str[i] >= 'A' && another.m_str[i] <= 'Z')
					? uint8_t(another.m_str[i] - 'A' + 'a') : uint8_t(another.m_str[i]);

			if (first < second) {
				return -1;
			} else if (first > second) {
				return 1;
			}
		}

		if (this->m_len < another.m_len) {
			return -1;
		} else if (this->m_len > another.m_len) {
			return 1;
		}

		return 0;
	}

	bool operator==(const fast_str_t& another) const {
		return this->cmp(another) == 0;
	}

	bool operator!=(const fast_str_t& another) const {
		return this->cmp(another) != 0;
	}

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


