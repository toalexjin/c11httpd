/**
 * Client connection event.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn_session.h"


namespace c11httpd {

enum event_result_t {
	// Close the connection.
	//
	// If this flag is on and there are pending data to send,
	// then it will send data before closing connection.
	event_result_disconnect = 1,

	// There are more data to send.
	//
	// When this flag is on, acceptor_t will keep calling
	// conn_event_t::get_more_data() until this flag becomes off.
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
	// "on_connected" event and "on_disconneted" event are always in pairs.
	//
	// "send_buf" might have some data pending to send,
	// so NEVER remove any data that the object already has,
	// and ALWAYS append data at the end.
	//
	// @return A combination value of event_result_t.
	virtual uint32_t on_connected(conn_session_t& session, buf_t& send_buf) = 0;

	// Connection was disconnected.
	//
	// "on_connected" event and "on_disconneted" event are always in pairs.
	virtual void on_disconnected(conn_session_t& session) = 0;

	// New data was received.
	//
	// -# If "recv_buf" is completely processed, then call "recv_buf->clear()"
	//    before this function returns.
	// -# If "recv_buf" is partially processed, then call "recv_buf->erase_front()"
	//    before this function returns.
	// -# If "recv_buf" is NOT processed, then do NOT clear its content.
	//    When "on_received" event is triggered next time,
	//    the unprocessed data is still in "recv_buf".
	//
	// "send_buf" might have some data pending to send,
	// so NEVER remove any data that the object already has,
	// and ALWAYS append data at the end.
	//
	// @return A combination value of event_result_t.
	virtual uint32_t on_received(conn_session_t& session,
			buf_t& recv_buf, buf_t& send_buf) = 0;

	// Get more data to send.
	//
	// "send_buf" might have some data pending to send,
	// so NEVER remove any data that the object already has,
	// and ALWAYS append data at the end.
	//
	// @return A combination value of event_result_t.
	virtual uint32_t get_more_data(conn_session_t& session, buf_t& send_buf) = 0;
};


} // namespace c11httpd.

