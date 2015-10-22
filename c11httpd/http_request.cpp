/**
 * HTTP request.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_request.h"


namespace c11httpd {


http_request_t::parse_result_t http_request_t::continue_to_parse(
	const buf_t* recv_buf, size_t* bytes) {

	return parse_result_t::ok;
}


} // namespace c11httpd.

