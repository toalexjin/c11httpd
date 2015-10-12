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

	this->m_ip.clear();
	this->m_sd.close();
	this->m_port = 0;
	this->m_ipv6 = false;
	this->m_recv_buf.clear();
	this->m_send_buf.clear();
	this->m_send_offset = 0;
	this->m_last_event_result = 0;
}

int conn_t::fd() const {
	return this->m_sd.get();
}

const std::string& conn_t::ip() const {
	return this->m_ip;
}

uint16_t conn_t::port() const {
	return this->m_port;
}

bool conn_t::ipv6() const {
	return this->m_ipv6;
}

buf_t* conn_t::recv_buf() {
	return &this->m_recv_buf;
}

buf_t* conn_t::send_buf() {
	return &this->m_send_buf;
}

err_t conn_t::recv(size_t* new_recv_size, bool* peer_closed) {
	err_t ret;
	const size_t unit_size = 1024;
	size_t ok_bytes;

	assert(new_recv_size != 0);
	assert(peer_closed != 0);

	*new_recv_size = 0;
	*peer_closed = false;

	this->m_recv_buf.back(unit_size);
	while (1) {
		ret = this->sock().recv(this->m_recv_buf.back(), this->m_recv_buf.free_size(), &ok_bytes);
		if (!ret) {
			if (ret == EAGAIN || ret == EWOULDBLOCK) {
				ret.set_ok();
			}

			break;
		}

		if (ok_bytes == 0) {
			*peer_closed = true;
			break;
		}

		*new_recv_size += ok_bytes;
		this->m_recv_buf.add_size(ok_bytes);

		// If no free buffer any more, it probably means there are more data to read.
		// Otherwise, it should be no more data to read and we do not need to re-allocate buffer.
		// Anyway, we should not stop until getting EAGAIN or EWOULDBLOCK.
		if (this->m_recv_buf.free_size() == 0) {
			this->m_recv_buf.back(unit_size);
		}
	}

	// If there are free space, then add a null-terminal to make debug easier.
	if (this->m_recv_buf.free_size() > 0) {
		this->m_recv_buf.back()[0] = 0;
	}

	return ret;
}

err_t conn_t::send(size_t* new_send_size) {
	err_t ret;
	size_t ok_bytes;

	assert(new_send_size != 0);
	*new_send_size = 0;

	if (this->m_send_offset == this->m_send_buf.size()) {
		return ret;
	}

	while (true) {
		ret = this->sock().send(this->m_send_buf.front() + this->m_send_offset,
				this->m_send_buf.size() - this->m_send_offset, &ok_bytes);
		if (!ret) {
			break;
		}

		*new_send_size += ok_bytes;
		this->m_send_offset += ok_bytes;

		// If all data has been sent, then reset state.
		if (this->m_send_offset == this->m_send_buf.size()) {
			this->m_send_offset = 0;
			this->m_send_buf.clear();
			break;
		}
	}

	return ret;
}


} // namespace c11httpd.


