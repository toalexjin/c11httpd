/**
 * RESTFul API result.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {

// RESTFul API result.
enum class rest_result_t {
	// The request has been fully processed
	// and its result is saved to "response".
	done = 0,

	// A critical error happened, no need to
	// send response back to client side
	// and the connection needs to close immediately.
	abandon = 1
};

} // namespace c11httpd.

