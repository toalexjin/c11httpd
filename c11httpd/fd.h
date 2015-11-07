/**
 * File descriptor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/err.h"


namespace c11httpd {

// A lightweight file descriptor class.
//
// Note that this object will be saved in STL containers,
// which might copy objects internally when re-allocating memory.
// Therefore, this object will not close file handle automatically in destructor.
class fd_t {
public:
	fd_t() : m_fd(-1) {
	}

	fd_t(int fd) : m_fd(fd) {
	}

	// We do not close file handle in destructor!!
	~fd_t() = default;

	fd_t(const fd_t&) = default;
	fd_t& operator=(const fd_t&) = default;

	fd_t& operator=(int fd) {
		return this->set(fd);
	}

	bool is_open() const {
		return this->m_fd >= 0;
	}

	bool closed() const {
		return !this->is_open();
	}

	int get() const {
		return this->m_fd;
	}

	// fd_t::set() is often used along with Linux system APIs,
	// so it's very important to keep "errno" no change
	// after this function returns.
	fd_t& set(int fd) {
		this->m_fd = fd;
		return *this;
	}

	err_t close();

	bool nonblock() const;
	err_t nonblock(bool flag);

	bool cloexec() const;
	err_t cloexec(bool flag);

	// Non-block read.
	err_t read_nonblock(buf_t* read_buf, size_t* new_read_size, bool* eof);

private:
	int m_fd;
};

} // namespace c11httpd.


