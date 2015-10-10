/**
 * Buffer.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"

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

	size_t free_size() const {
		return this->m_capacity - this->m_size;
	}

	uint8_t* front() const {
		return this->m_buf;
	}

	uint8_t* back() const {
		return this->m_buf + this->m_size;
	}

	void* back(size_t free_size);

	void erase_front(size_t erased_size);
	void erase_back(size_t erased_size);

private:
	buf_t(const buf_t&) = delete;
	buf_t& operator=(const buf_t&) = delete;

private:
	uint8_t* m_buf;
	size_t m_capacity;
	size_t m_size;
};

} // namespace c11httpd.


