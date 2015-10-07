/**
 * Buffer.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <cstring>
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
		this->m_buf = 0;
		this->m_capacity = 0;
		this->m_size = 0;
	}

	~buf_t() {
		if (this->m_buf != 0) {
			::operator delete((void*) m_buf);
			this->m_buf = 0;
		}

		this->m_capacity = 0;
		this->m_size = 0;
	}

	size_t capacity() const {
		return this->m_capacity;
	}

	size_t size() const {
		return this->m_size;
	}

	size_t pending_size() const {
		return this->m_capacity - this->m_size;
	}

	void* get() const {
		return this->m_buf;
	}

	void* pending() const {
		return this->m_buf + this->m_size;
	}

	void* pending(size_t pending_size) {
		if (this->m_capacity - this->m_size < pending_size) {
			size_t new_capacity = this->m_capacity * 2;

			if (new_capacity - this->m_size < pending_size) {
				new_capacity = this->m_size + pending_size;
			}

			auto new_buf = (uint8_t*)::operator new(new_capacity);
			std::memcpy(new_buf, this->m_buf, this->m_size);

			::operator delete((void*) this->m_buf);
			this->m_buf = new_buf;
			this->m_capacity = new_capacity;
		}

		return this->m_buf + this->m_size;
	}

	/**
	 * Clear content.
	 *
	 * We do not free memory so that it could be re-used.
	 */
	void clear() {
		this->m_size = 0;
	}

private:
	buf_t(const buf_t&) = delete;
	buf_t& operator=(const buf_t&) = delete;

private:
	uint8_t* m_buf;
	size_t m_capacity;
	size_t m_size;
};

} // namespace c11httpd.


