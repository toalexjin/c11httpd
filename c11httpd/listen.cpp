/**
 * Connection base class.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/listen.h"


namespace c11httpd {


listen_t::~listen_t() {
	this->close();
}

void listen_t::close() {
	this->m_sd.close();
}

} // namespace c11httpd.

