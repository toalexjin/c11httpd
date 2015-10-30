/**
 * Utility functions.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {


// Utility functions.
class utility_t {
public:
	enum {
		// Response date length (including null terminal)
		response_date_len = 32
	};

public:
	// Get current GMT time for HTTP response header "Date:???".
	static void response_date(char* str);

private:
	utility_t() = delete;
};


} // namespace c11httpd.

