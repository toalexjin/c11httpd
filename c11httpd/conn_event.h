/**
 * Client connection event.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn_session.h"


namespace c11httpd {


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
	// If this function returns false, then the new connection
	// would be closed by the library immediately and "on_disconnected"
	// event would NOT be invoked.
	virtual bool on_connected(conn_session_t* session) = 0;

	// Connection was disconnected.
	//
	// For each-success "on_connected" call, there MUST
	// be a corresponding "on_disconnected" event invoked.
	// This rule is guaranteed by the library.
	virtual void on_disconnected(conn_session_t* session) = 0;

	// New data was received.
	virtual void on_received(conn_session_t* session) = 0;
};

}

