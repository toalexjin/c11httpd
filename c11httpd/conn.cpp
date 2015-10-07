/**
 * TCP connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/conn.h"


namespace c11httpd {


conn_t::~conn_t() {
	this->close();
}

void conn_t::close() {
	conn_base_t::close();

	this->m_recv.clear();
	this->m_send.clear();
}

err_t conn_t::recv(size_t* new_recv_size, bool* peer_closed) {
	err_t ret;
	size_t ok_bytes;

	assert(new_recv_size != 0);
	assert(peer_closed != 0);

	*new_recv_size = 0;
	*peer_closed = false;

	while (1) {
		m_recv.back(1024);

		ret = this->sock().recv(m_recv.back(), m_recv.free_size(), &ok_bytes);
		if (!ret) {
			break;
		}

		if (ok_bytes == 0) {
			*peer_closed = true;
			break;
		}

		*new_recv_size += ok_bytes;
	}

	return ret;
}


} // namespace c11httpd.


