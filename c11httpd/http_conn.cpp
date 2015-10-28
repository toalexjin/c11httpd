/**
 * HTTP connection.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_conn.h"


namespace c11httpd {


void http_conn_t::clear() {
	this->m_request.clear();
	this->m_response.clear();
	this->m_placeholders.clear();
}


} // namespace c11httpd.

