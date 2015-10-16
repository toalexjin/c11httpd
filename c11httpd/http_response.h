/**
 * HTTP Response.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <string>


namespace c11httpd {


// HTTP Response.
class http_response_t {
public:
	http_response_t() = default;
	virtual ~http_response_t() = default;


private:
};


} // namespace c11httpd.

