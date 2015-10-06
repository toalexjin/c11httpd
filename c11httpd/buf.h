/**
 * Buffer.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <new>

namespace c11httpd {


/**
 * Buffer.
 *
 * conn_t uses this buffer object to save send & recv data.
 */
class buf_t {
public:
	buf_t() {
		this->m_data = 0;
		this->m_capacity = 0;
		this->m_first = 0;
		this->m_last = 0;
	}

	~buf_t() {
		if (this->m_data != 0) {
			::operator delete((void*) m_data);
			this->m_data = 0;
		}

		this->m_capacity = 0;
		this->m_first = 0;
		this->m_last = 0;
	}

	size_t capacity() const {
		return this->m_capacity;
	}

	size_t size() const {
		return size_t(this->m_last - this->m_first);
	}

	/**
	 * Clear content.
	 *
	 * Note that we do not free memory, because we wnat
	 * the object could be re-used.
	 */
	void clear() {
		this->m_first = 0;
		this->m_last = 0;
	}

private:
	buf_t(const buf_t&) = delete;
	buf_t& operator=(const buf_t&) = delete;

private:
	unsigned char* m_data;
	size_t m_capacity;
	size_t m_first;
	size_t m_last;
};

} // namespace c11httpd.


