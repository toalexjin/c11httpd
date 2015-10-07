/**
 * Buffer.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/buf.h"
#include <cstring>
#include <new>


namespace c11httpd {


buf_t::~buf_t() {
	if (this->m_buf != 0) {
		::operator delete((void*) m_buf);
		this->m_buf = 0;
	}

	this->m_capacity = 0;
	this->m_size = 0;
}

void* buf_t::pending(size_t pending_size) {
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

void buf_t::erase_front(size_t removed_size) {
	assert(removed_size < this->m_size);

	std::memmove(this->m_buf, this->m_buf + removed_size, this->m_size - removed_size);
	this->m_size -= removed_size;
}

void buf_t::erase_back(size_t removed_size) {
	assert(removed_size < this->m_size);

	this->m_size -= removed_size;
}


} // namespace c11httpd.


