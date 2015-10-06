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


} // namespace c11httpd.


