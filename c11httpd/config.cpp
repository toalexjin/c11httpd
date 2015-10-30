/**
 * TCP/HTTP Server Configuration.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/pre__.h"
#include "c11httpd/config.h"


namespace c11httpd {


config_t::config_t() {
	this->set_default();
}

void config_t::set_default() {
	this->m_flags = keep_alive | response_date;
	this->m_worker_processes = 0;
	this->m_backlog = 10;
	this->m_max_epoll_events = 256;
	this->m_max_free_connection = 128;
}


} // namespace c11httpd.

