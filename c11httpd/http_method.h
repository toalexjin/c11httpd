/**
 * HTTP Method.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {

// HTTP Method.
enum http_method_t {
	http_method_get,
	http_method_post,
	http_method_put,
	http_method_delete
};


}

