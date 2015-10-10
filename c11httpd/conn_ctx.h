/**
 * Client session context, defined & used by caller.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {


// Client session context, defined & used by user.
//
// User could create their own context object
// associated with each conn_session_t object.
class conn_ctx_t {
public:
	conn_ctx_t() = default;
	virtual ~conn_ctx_t() = default;

	// Clear content.
	//
	// When "on_disconnected" event completes, this function would be called
	// by the library automatically, and the conn_ctx_t would NOT be free'd,
	// which will be re-used by later incoming connections.
	virtual void clear() = 0;
};

}

