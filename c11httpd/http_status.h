/**
 * HTTP Status.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {

// HTTP Status.
enum http_status_t {
	http_status_ok = 200,

	http_status_not_found = 404
};


}

