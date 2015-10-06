/**
 * Connection base class.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/conn_base.h"


namespace c11httpd {


conn_base_t::~conn_base_t() {
	this->close();
}

void conn_base_t::close() {
	this->m_fd.close();
}


} // namespace c11httpd.


