/**
 * Client connection event.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn_session.h"


namespace c11httpd {

enum {
	// Close the connection.
	event_result_disconnect = 1,

	// There are more data to send.
	//
	// "on_received" might return this flag.
	event_result_more_data = (1 << 1)
};

// Client connection event.
//
// This is client connection event interface. User should create
// a sub-class that implements this interface in order to handle
// client connection events.
class conn_event_t {
public:
	conn_event_t() = default;
	virtual ~conn_event_t() = default;

	// A new connection was established.
	//
	// If event_result_disconnect is returned, then the connection
	// would be closed and "on_disconnected" event would NOT be triggered.
	//
	// @return zero or event_result_disconnect.
	virtual uint32_t on_connected(conn_session_t* session, buf_t* send_buf);

	// Connection was disconnected.
	//
	// For each-success "on_connected" call, there MUST
	// be a corresponding "on_disconnected" event triggered.
	// This rule is guaranteed by the library.
	virtual void on_disconnected(conn_session_t* session);

	// New data was received.
	//
	// @return A combination value of event_result_???
	virtual uint32_t on_received(conn_session_t* session,
			buf_t* recv_buf, buf_t* send_buf);

	// Get more data to send.
	virtual uint32_t get_more_data(conn_session_t* session, buf_t* send_buf);
};

} // namespace c11httpd.

