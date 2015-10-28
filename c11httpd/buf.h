/**
 * Buffer.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/fast_str.h"
#include <string>
#include <cstring>


namespace c11httpd {


// Buffer.
//
// conn_t uses this buffer object to save send & recv data.
class buf_t {
public:
	buf_t() {
		this->m_buf = 0;
		this->m_capacity = 0;
		this->m_size = 0;
	}

	~buf_t();

	// Clear content.
	//
	// We do not free memory so that the buffer could be re-used.
	void clear() {
		this->m_size = 0;
	}

	size_t capacity() const {
		return this->m_capacity;
	}

	size_t size() const {
		return this->m_size;
	}

	void size(size_t new_size) {
		assert(new_size <= this->m_capacity);

		this->m_size = new_size;
	}

	void add_size(size_t added_size) {
		assert(this->m_size + added_size <= this->m_capacity);

		this->m_size += added_size;
	}

	size_t free_size() const {
		return this->m_capacity - this->m_size;
	}

	char* front() const {
		return this->m_buf;
	}

	char* back() const {
		return this->m_buf + this->m_size;
	}

	char* back(size_t free_size);

	buf_t& push_back(const void* data, size_t size);
	buf_t& push_back(const std::string& str);
	buf_t& push_back(const fast_str_t& str);
	buf_t& push_back(const char* str);
	buf_t& push_back(const buf_t& another);
	buf_t& push_back(int number);
	buf_t& push_back(unsigned int number);

	buf_t& operator<<(const std::string& str) {
		return this->push_back(str);
	}

	buf_t& operator<<(const fast_str_t& str) {
		return this->push_back(str);
	}

	buf_t& operator<<(const char* str) {
		return this->push_back(str);
	}

	buf_t& operator<<(const buf_t& another) {
		return this->push_back(another);
	}

	buf_t& operator<<(int number) {
		return this->push_back(number);
	}

	buf_t& operator<<(unsigned int number) {
		return this->push_back(number);
	}

	void erase_front(size_t erased_size);
	void erase_back(size_t erased_size);

	const char& operator[](size_t index) const {
		return this->at(index);
	}

	char& operator[](size_t index) {
		return this->at(index);
	}

	const char& at(size_t index) const {
		assert(index < this->m_size);
		return this->m_buf[index];
	}

	char& at(size_t index) {
		assert(index < this->m_size);
		return this->m_buf[index];
	}

private:
	template <class T>
	buf_t& push_back_integer(const char* format, T n) {
		char buf[32];

		const int str_len = std::snprintf(buf, sizeof(buf), format, n);
		if (str_len >= 0 && str_len <= sizeof(buf)) {
			this->push_back(buf, str_len);
		}

		return *this;
	}

private:
	buf_t(const buf_t&) = delete;
	buf_t& operator=(const buf_t&) = delete;

private:
	char* m_buf;
	size_t m_capacity;
	size_t m_size;
};


} // namespace c11httpd.


