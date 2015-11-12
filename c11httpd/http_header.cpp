/**
 * HTTP header.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_header.h"
#include <algorithm>


namespace c11httpd {


const fast_str_t http_header_t::Accept = "Accept";
const fast_str_t http_header_t::Accept_Charset = "Accept-Charset";
const fast_str_t http_header_t::Accept_Encoding = "Accept-Encoding";
const fast_str_t http_header_t::Accept_Language = "Accept-Language";

const fast_str_t http_header_t::Accept_Ranges = "Accept-Ranges";
const fast_str_t http_header_t::App_Json_UTF8 = "application/json; charset=UTF-8";
const fast_str_t http_header_t::App_Octet_Stream = "application/octet-stream";
const fast_str_t http_header_t::App_RPM = "application/x-redhat-package-manager";
const fast_str_t http_header_t::App_XML_UTF8 = "application/xml; charset=UTF-8";
const fast_str_t http_header_t::App_ZIP = "application/zip";
const fast_str_t http_header_t::Attachment = "attachment";
const fast_str_t http_header_t::Authorization = "Authorization";
const fast_str_t http_header_t::Bytes = "bytes";
const fast_str_t http_header_t::Compress = "compress";
const fast_str_t http_header_t::Connection = "Connection";

const fast_str_t http_header_t::Content_Language = "Content-Language";
const fast_str_t http_header_t::Content_Location = "Content-Location";
const fast_str_t http_header_t::Content_MD5 = "Content-MD5";
const fast_str_t http_header_t::Content_Disposition = "Content-Disposition";
const fast_str_t http_header_t::Content_Encoding = "Content-Encoding";
const fast_str_t http_header_t::Content_Length = "Content-Length";
const fast_str_t http_header_t::Content_Range = "Content-Range";
const fast_str_t http_header_t::Content_Type = "Content-Type";

const fast_str_t http_header_t::Date = "Date";
const fast_str_t http_header_t::Deflate = "deflate";
const fast_str_t http_header_t::Filename = "filename";
const fast_str_t http_header_t::GZIP = "gzip";
const fast_str_t http_header_t::Host = "Host";
const fast_str_t http_header_t::HTTP_VERSION_1_1 = "HTTP/1.1";
const fast_str_t http_header_t::Identify = "identify";
const fast_str_t http_header_t::Image_GIF = "image/gif";
const fast_str_t http_header_t::Image_JPEG = "image/jpeg";
const fast_str_t http_header_t::Image_PNG = "image/png";
const fast_str_t http_header_t::Keep_Alive = "keep-alive";
const fast_str_t http_header_t::Last_Modified = "Last-Modified";
const fast_str_t http_header_t::Location = "Location";
const fast_str_t http_header_t::Range = "Range";
const fast_str_t http_header_t::Server = "Server";
const fast_str_t http_header_t::Text_CSS_UTF8 = "text/css; charset=UTF-8";
const fast_str_t http_header_t::Text_HTML_UTF8 = "text/html; charset=UTF-8";
const fast_str_t http_header_t::Text_JavaScript = "text/javascript";
const fast_str_t http_header_t::Text_Plain_UTF8 = "text/plain; charset=UTF-8";


// Single instance.
http_header_t::all http_header_t::all::st_instance;


http_header_t::all::all() {
	// Common fields.
	const std::vector<fast_str_t> common = {
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
	const std::vector<fast_str_t> request = {
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
	const std::vector<fast_str_t> response = {
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
	const std::vector<fast_str_t> entity = {
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
	this->m_request.reserve(
		common.size() + request.size() + entity.size());

	for (const auto& item : common) {
		this->m_request.push_back(item);
	}

	for (const auto& item : request) {
		this->m_request.push_back(item);
	}

	for (const auto& item : entity) {
		this->m_request.push_back(item);
	}

	// Make the vector ascending sorted.
	std::sort(this->m_request.begin(),
			this->m_request.end(),
			m_less
	);

	// Add common, response, entity fields.
	this->m_response.reserve(
		common.size() + response.size() + entity.size());

	for (const auto& item : common) {
		this->m_response.push_back(item);
	}

	for (const auto& item : response) {
		this->m_response.push_back(item);
	}

	for (const auto& item : entity) {
		this->m_response.push_back(item);
	}

	// Make the vector ascending sorted.
	std::sort(this->m_response.begin(),
			this->m_response.end(),
			m_less
	);
}

const fast_str_t* http_header_t::all::request_find(const fast_str_t& key) const {
	const auto it = std::lower_bound(
			this->m_request.cbegin(),
			this->m_request.cend(),
			key,
			m_less);

	if (it != this->m_request.cend() && !m_less(key, *it)) {
		return &(*it);
	} else {
		return 0;
	}
}

const fast_str_t* http_header_t::all::response_find(const fast_str_t& key) const {
	const auto it = std::lower_bound(
			this->m_response.cbegin(),
			this->m_response.cend(),
			key,
			m_less);

	if (it != this->m_response.cend() && !m_less(key, *it)) {
		return &(*it);
	} else {
		return 0;
	}
}


} // namespace c11httpd.

