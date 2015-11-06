/**
 * Socket.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/fd.h"
#include <unistd.h>
#include <fcntl.h>


namespace c11httpd {


err_t fd_t::close() {
	err_t ret;

	if (!this->closed()) {
		if (::close(this->m_fd) == 0) {
			this->m_fd = -1;
		} else {
			ret.set_current();
		}
	}

	return ret;
}

bool fd_t::nonblock() const {
	assert(this->is_open());

	const int value = fcntl(this->get(), F_GETFL);
	return (value & O_NONBLOCK) != 0;
}

err_t fd_t::nonblock(bool flag) {
	assert(this->is_open());

	const int old = fcntl(this->get(), F_GETFL);
	const int updated = flag ? (old | O_NONBLOCK) : (old & (~O_NONBLOCK));

	if (old != updated && fcntl(this->get(), F_SETFL, updated) != 0) {
		return err_t::current();
	}

	return err_t();
}

bool fd_t::cloexec() const {
	assert(this->is_open());

	const int value = fcntl(this->get(), F_GETFD);
	return (value & FD_CLOEXEC) != 0;
}

err_t fd_t::cloexec(bool flag) {
	assert(this->is_open());

	const int old = fcntl(this->get(), F_GETFD);
	const int updated = flag ? (old | FD_CLOEXEC) : (old & (~FD_CLOEXEC));

	if (old != updated && fcntl(this->get(), F_SETFD, updated) != 0) {
		return err_t::current();
	}

	return err_t();
}


} // namespace c11httpd.


