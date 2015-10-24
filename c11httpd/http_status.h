/**
 * HTTP Status.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {


// HTTP Status.
class http_status_t {
public:
	static const int ok = 200;
	static const int not_found = 404;
};


}

