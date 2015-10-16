/**
 * HTTP Request.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <string>


namespace c11httpd {


// HTTP Request.
class http_request_t {
public:
	http_request_t() = default;
	virtual ~http_request_t() = default;


private:
};


} // namespace c11httpd.

