/**
 * TCP connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/conn.h"
#include <errno.h>


namespace c11httpd {


conn_t::~conn_t() {
	this->close();
}

void conn_t::close() {
	// Clear context but do not free memory
	// so that the context could be re-used later.
	if (this->get_ctx() != 0) {
		this->get_ctx()->clear();
	}

	this->m_recv.clear();
	this->m_send.clear();

	conn_base_t::close();
}

const std::string& conn_t::ip() const {
	return conn_base_t::ip();
}

uint16_t conn_t::port() const {
	return conn_base_t::port();
}

bool conn_t::ipv6() const {
	return conn_base_t::ipv6();
}

buf_t* conn_t::recv_buf() {
	return &this->m_recv;
}

buf_t* conn_t::send_buf() {
	return &this->m_send;
}

err_t conn_t::recv(size_t* new_recv_size, bool* peer_closed) {
	err_t ret;
	const size_t unit_size = 1024;
	size_t ok_bytes;

	assert(new_recv_size != 0);
	assert(peer_closed != 0);

	*new_recv_size = 0;
	*peer_closed = false;

	this->m_recv.back(unit_size);
	while (1) {
		ret = this->sock().recv(this->m_recv.back(), this->m_recv.free_size(), &ok_bytes);
		if (!ret) {
			if (*new_recv_size > 0 && (ret == EAGAIN || ret == EWOULDBLOCK)) {
				ret.set_ok();
			}

			break;
		}

		if (ok_bytes == 0) {
			*peer_closed = true;
			break;
		}

		*new_recv_size += ok_bytes;
		this->m_recv.size(this->m_recv.size() + ok_bytes);

		// If no free buffer any more, it probably means there are more data to read.
		// Otherwise, it should be no more data to read and we do not need to re-allocate buffer.
		// Anyway, we should not stop until getting EAGAIN or EWOULDBLOCK.
		if (this->m_recv.free_size() == 0) {
			this->m_recv.back(unit_size);
		}
	}

	// If there are free space, then add a null-terminal to make debug easier.
	if (this->m_recv.free_size() > 0) {
		this->m_recv.back()[0] = 0;
	}

	return ret;
}

err_t conn_t::send(size_t* new_send_size) {
	err_t ret;
	size_t ok_bytes;

	assert(new_send_size != 0);
	*new_send_size = 0;

	while (this->m_send_offset < this->m_send.size()) {
		ret = this->sock().send(this->m_send.front() + this->m_send_offset,
				this->m_send.size() - this->m_send_offset, &ok_bytes);
		if (!ret) {
			if (*new_send_size > 0 && (ret == EAGAIN || ret == EWOULDBLOCK)) {
				ret.set_ok();
			}

			break;
		}

		*new_send_size += ok_bytes;
		this->m_send_offset += ok_bytes;
	}

	return ret;
}


} // namespace c11httpd.


