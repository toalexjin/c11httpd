/**
 * File descriptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"


namespace c11httpd {

/**
 * A lightweight file descriptor class.
 *
 * Note that this object will be saved in STL containers,
 * which might copy objects internally when re-allocating memory.
 * Therefore, this object will not close file handle automatically in destructor.
 */
class fd_t {
public:
	fd_t() : m_handle(-1) {
	}

	fd_t(int handle) : m_handle(handle) {
	}

	// We do not close file handle in destructor!!
	~fd_t() = default;

	fd_t(const fd_t&) = default;
	fd_t& operator=(const fd_t&) = default;

	fd_t& operator=(int handle) {
		return this->set(handle);
	}

	bool is_opened() const {
		return this->m_handle >= 0;
	}

	bool is_closed() const {
		return !this->is_opened();
	}

	int get() const {
		return this->m_handle;
	}

	// fd_t::set() is often used along with Linux system APIs,
	// so it's very important to keep "errno" no change
	// after this function returns.
	fd_t& set(int handle) {
		this->m_handle = handle;
		return *this;
	}

	err_t close();

private:
	int m_handle;
};

} // namespace c11httpd.


