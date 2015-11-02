/**
 * TCP connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/conn.h"
#include <errno.h>
#include <signal.h>


namespace c11httpd {


conn_t::~conn_t() {
	this->close();
}

void conn_t::close() {
	// Clear context but do not free memory
	// so that the context could be re-used later.
	if (this->ctx() != 0) {
		this->ctx()->clear();
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

const std::string& conn_t::ip() const {
	return this->m_ip;
}

uint16_t conn_t::port() const {
	return this->m_port;
}

bool conn_t::ipv6() const {
	return this->m_ipv6;
}

buf_t& conn_t::recv_buf() {
	return this->m_recv_buf;
}

buf_t& conn_t::send_buf() {
	return this->m_send_buf;
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

err_t conn_t::aio_read(fd_t fd, int64_t offset,
	char* buf, size_t size, int64_t* id) {

	assert(fd.is_open());
	assert(offset >= 0);
	assert(buf != 0 || size == 0);
	assert(id != 0);

	err_t ret;
	aio_node_t* node = new aio_node_t();

	*id = (++ m_aio_sequence);
	node->m_id = *id;
	node->m_cb.aio_fildes = fd.get();
	node->m_cb.aio_offset = offset;
	node->m_cb.aio_buf = (void*) buf;
	node->m_cb.aio_nbytes = size;

	node->m_cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	node->m_cb.aio_sigevent.sigev_signo = SIGIO;
	node->m_cb.aio_sigevent.sigev_value.sival_ptr = this;

	this->m_aio_running[node->m_id] = std::unique_ptr<aio_node_t>(node);

	if (::aio_read(&node->m_cb) != 0) {
		ret.set_current();

		this->m_aio_running.erase(node->m_id);
		delete node;
		node = 0;

		*id = 0;
	}

	return ret;
}

err_t conn_t::aio_write(fd_t fd, int64_t offset,
	const char* buf, size_t size, int64_t* id) {

	assert(fd.is_open());
	assert(offset >= 0);
	assert(buf != 0 || size == 0);
	assert(id != 0);

	err_t ret;
	aio_node_t* node = new aio_node_t();

	*id = (++ m_aio_sequence);
	node->m_id = *id;
	node->m_cb.aio_fildes = fd.get();
	node->m_cb.aio_offset = offset;
	node->m_cb.aio_buf = (void*) buf;
	node->m_cb.aio_nbytes = size;

	node->m_cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	node->m_cb.aio_sigevent.sigev_signo = SIGIO;
	node->m_cb.aio_sigevent.sigev_value.sival_ptr = this;

	this->m_aio_running[node->m_id] = std::unique_ptr<aio_node_t>(node);

	if (::aio_write(&node->m_cb) != 0) {
		ret.set_current();

		this->m_aio_running.erase(node->m_id);
		delete node;
		node = 0;

		*id = 0;
	}

	return ret;
}

err_t conn_t::aio_cancel(int64_t id) {
	err_t ret;

	return ret;
}

void conn_t::aio_completed(std::vector<aio_t>* completed) {
	assert(completed != 0);

	// Clear content.
	completed->clear();

	aio_t pub;
	for (const auto& item : this->m_aio_completed) {
		item.second->to_pub(&pub);
		completed->push_back(pub);
	}

	this->m_aio_completed.clear();
}


} // namespace c11httpd.


