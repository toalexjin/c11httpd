/**
 * HTTP header fields.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_header_fields.h"
#include <algorithm>


namespace c11httpd {


// Single instance.
http_header_fields_t http_header_fields_t::st_instance;


http_header_fields_t::http_header_fields_t() {
	// Common fields.
	const std::vector<fast_str_t> common_fields = {
		"Cache-Control",
		"Connection",
		"Date",
		"Pragma",
		"Trailer",
		"Transfer-Encoding",
		"Upgrade",
		"Via",
		"Warning"
	};

	// Request fields.
	const std::vector<fast_str_t> request_fields = {
		"Accept",
		"Accept-Charset",
		"Accept-Encoding",
		"Accept-Language",
		"Authorization",
		"Expect",
		"From",
		"Host",
		"If-Match",
		"If-Modified-Since",
		"If-None-Match",
		"If-Range",
		"If-Unmodified-Since",
		"Max-Forwards",
		"Proxy-Authorization",
		"Range",
		"Referer",
		"TE",
		"User-Agent"
	};

	// Response fields.
	const std::vector<fast_str_t> response_fields = {
		"Accept-Ranges",
		"Age",
		"ETag",
		"Location",
		"Proxy-Authenticate",
		"Retry-After"
		"Server",
		"Vary",
		"WWW-Authenticate"
	};

	// Entity fields.
	const std::vector<fast_str_t> entity_fields = {
		"Allow",
		"Content-Encoding",
		"Content-Language",
		"Content-Length",
		"Content-Location",
		"Content-MD5",
		"Content-Range",
		"Content-Type",
		"Expires",
		"Last-Modified"
	};

	// Add common, request, entity fields.
	this->m_request_fields.reserve(
		common_fields.size() + request_fields.size() + entity_fields.size());

	for (const auto& item : common_fields) {
		this->m_request_fields.push_back(item);
	}

	for (const auto& item : request_fields) {
		this->m_request_fields.push_back(item);
	}

	for (const auto& item : entity_fields) {
		this->m_request_fields.push_back(item);
	}

	// Make the vector ascending sorted.
	std::sort(this->m_request_fields.begin(),
			this->m_request_fields.end(),
			fast_str_less_nocase_t()
	);

	// Add common, response, entity fields.
	this->m_response_fields.reserve(
		common_fields.size() + response_fields.size() + entity_fields.size());

	for (const auto& item : common_fields) {
		this->m_response_fields.push_back(item);
	}

	for (const auto& item : response_fields) {
		this->m_response_fields.push_back(item);
	}

	for (const auto& item : entity_fields) {
		this->m_response_fields.push_back(item);
	}

	// Make the vector ascending sorted.
	std::sort(this->m_response_fields.begin(),
			this->m_response_fields.end(),
			fast_str_less_nocase_t()
	);

}


} // namespace c11httpd.

