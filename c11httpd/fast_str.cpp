/**
 * Fast string.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/fast_str.h"
#include <algorithm>


namespace c11httpd {

const fast_str_t fast_str_t::empty_string = fast_str_t();


int fast_str_t::cmp(const fast_str_t& another) const {
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

int fast_str_t::cmpi(const fast_str_t& another) const {
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

size_t fast_str_t::split(const char* delims, std::vector<fast_str_t>* items) const {
	assert(delims != 0 && *delims != 0);

	// Clear content.
	items->clear();

	auto start = this->find_first_not_of(delims);
	while (start != npos) {
		auto end = this->find_first_of(delims, start + 1);
		if (end == npos) {
			items->push_back(this->substr(start));
			break;
		} else {
			items->push_back(this->substr(start, end - start));
			start = this->find_first_not_of(delims, end + 1);
		}
	}

	return items->size();
}

size_t fast_str_t::find_first_of(char ch, size_t pos) const {
	for (size_t i = pos; i < this->m_len; ++i) {
		if (ch == this->m_str[i]) {
			return i;
		}
	}

	return npos;
}

size_t fast_str_t::find_first_of(const char* delims, size_t pos) const {
	for (size_t i = pos; i < this->m_len; ++i) {
		for (const char* ptr = delims; *ptr != 0; ++ptr) {
			if (*ptr == this->m_str[i]) {
				return i;
			}
		}
	}

	return npos;
}

size_t fast_str_t::find_first_not_of(char ch, size_t pos) const {
	for (size_t i = pos; i < this->m_len; ++i) {
		if (ch != this->m_str[i]) {
			return i;
		}
	}

	return npos;
}

size_t fast_str_t::find_first_not_of(const char* delims, size_t pos) const {
	for (size_t i = pos; i < this->m_len; ++i) {
		const char* ptr = delims;

		while (*ptr != 0 && *ptr != this->m_str[i]) {
			++ptr;
		}

		if (*ptr == 0) {
			return i;
		}
	}

	return npos;
}

size_t fast_str_t::trim_left(const char* delims) {
	assert(delims != 0 && *delims != 0);

	const auto old_len = this->m_len;
	while (this->m_len > 0) {
		const char* ptr = delims;

		while (*ptr != 0 && *ptr != this->m_str[0]) {
			++ptr;
		}

		if (*ptr == 0) {
			break;
		}

		this->m_str++;
		this->m_len--;
	}

	return old_len - this->m_len;
}

size_t fast_str_t::trim_right(const char* delims) {
	assert(delims != 0 && *delims != 0);

	const auto old_len = this->m_len;
	for (size_t i = this->m_len; i > 0; --i) {
		const char* ptr = delims;

		while (*ptr != 0 && *ptr != this->m_str[i - 1]) {
			++ptr;
		}

		if (*ptr == 0) {
			break;
		}

		this->m_len--;
	}

	return old_len - this->m_len;
}

size_t fast_str_t::trim(const char* delims) {
	return this->trim_left(delims) + this->trim_right(delims);
}

fast_str_t fast_str_t::before(char ch) const {
	const auto pos = this->find_first_of(ch);
	if (pos == fast_str_t::npos) {
		return *this;
	} else {
		return this->substr(0, pos);
	}
}

fast_str_t fast_str_t::after(char ch) const {
	const auto pos = this->find_first_of(ch);
	if (pos == fast_str_t::npos) {
		return fast_str_t();
	} else {
		return this->substr(pos + 1);
	}
}

bool fast_str_t::to_number(int32_t* value) const {
	char buf[64];
	const auto len = std::min(sizeof(buf) - 1, this->m_len);

	assert(value != 0);

	std::strncpy(buf, m_str, len);
	buf[len] = 0;

	int tmp;
	if (std::sscanf(buf, "%d", &tmp) != 1) {
		*value = 0;
		return false;
	}

	*value = int32_t(tmp);
	return true;
}

bool fast_str_t::to_number(uint32_t* value) const {
	char buf[64];
	const auto len = std::min(sizeof(buf) - 1, this->m_len);

	assert(value != 0);

	std::strncpy(buf, m_str, len);
	buf[len] = 0;

	unsigned int tmp;
	if (std::sscanf(buf, "%u", &tmp) != 1) {
		*value = 0;
		return false;
	}

	*value = uint32_t(tmp);
	return true;
}

int32_t fast_str_t::to_i32(int32_t default_value) const {
	int32_t value;

	return this->to_number(&value) ? value : default_value;
}

uint32_t fast_str_t::to_u32(uint32_t default_value) const {
	uint32_t value;

	return this->to_number(&value) ? value : default_value;
}

bool fast_str_t::getline(fast_str_t* line, size_t pos, size_t* next_pos) {
	assert(line != 0);

	size_t end = this->find_first_of('\n', pos);
	if (end == npos) {
		line->clear();
		if (next_pos != 0) {
			*next_pos = npos;
		}
		return false;
	}

	if (next_pos != 0) {
		*next_pos = end + 1;
	}

	char* ptr = (char*) m_str;
	ptr[end] = 0;

	if (end > pos && ptr[end - 1] == '\r') {
		ptr[end - 1] = 0;
		-- end;
	}

	line->set(ptr + pos, end - pos);
	return true;
}


} // namespace c11httpd.

