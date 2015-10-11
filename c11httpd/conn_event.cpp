/**
 * Client connection event.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/conn_event.h"


namespace c11httpd {


uint32_t conn_event_t::on_connected(conn_session_t* session, buf_t* send_buf) {
	assert(session != 0);

	(void) &session;
	(void) &send_buf;
	return 0;
}

void conn_event_t::on_disconnected(conn_session_t* session) {
	assert(session != 0);

	(void) &session;
}

uint32_t conn_event_t::on_received(conn_session_t* session,
	buf_t* recv_buf, buf_t* send_buf) {
	assert(session != 0);

	(void) &session;
	(void) &send_buf;

	recv_buf->clear();
	return 0;
}

uint32_t conn_event_t::get_more_data(conn_session_t* session, buf_t* send_buf) {
	assert(session != 0);

	(void) &session;
	(void) &send_buf;

	return 0;
}


} // namespace c11httpd.

