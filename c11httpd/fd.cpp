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

err_t fd_t::read_nonblock(buf_t* read_buf, size_t* new_read_size, bool* eof) {
	err_t ret;
	const size_t unit_size = 1024;

	assert(new_read_size != 0);
	assert(eof != 0);

	*new_read_size = 0;
	*eof = false;

	read_buf->back(unit_size);
	while (1) {
		const int ok_bytes = ::read(this->m_fd, read_buf->back(), read_buf->free_size());
		if (ok_bytes == -1) {
			ret.set_current();
			if (ret == EAGAIN || ret == EWOULDBLOCK) {
				ret.set_ok();
			}

			break;
		}

		if (ok_bytes == 0) {
			*eof = true;
			break;
		}

		*new_read_size += ok_bytes;
		read_buf->add_size(ok_bytes);

		// If no free buffer any more, it probably means there are more data to read.
		// Otherwise, it should be no more data to read and we do not need to re-allocate buffer.
		// Anyway, we should not stop until getting EAGAIN or EWOULDBLOCK.
		if (read_buf->free_size() == 0) {
			read_buf->back(unit_size);
		}
	}

	// If there are free space, then add a null-terminal to make debug easier.
	if (read_buf->free_size() > 0) {
		read_buf->back()[0] = 0;
	}

	return ret;
}


} // namespace c11httpd.


