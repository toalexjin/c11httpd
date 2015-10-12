/**
 * Connection base class.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include <c11httpd/listen.h>


namespace c11httpd {


listen_t::~listen_t() {
	this->close();
}

void listen_t::close() {
	this->m_sd.close();
	this->m_sd = -1;
}

int listen_t::fd() const {
	return this->m_sd.get();
}

} // namespace c11httpd.


