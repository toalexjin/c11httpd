/**
 * Context object, defined & used by caller.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {


// Context object, defined & used by caller.
class ctx_t {
public:
	ctx_t() = default;
	virtual ~ctx_t() = default;

	// Clear content.
	//
	// A context object might be re-used, so this function
	// might just clear content but do not free memory
	// in order to enhance performance.
	virtual void clear() = 0;
};


} // namespace c11httpd.

