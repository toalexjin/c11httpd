/**
 * HTTP header.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/fast_str.h"
#include <iostream>


namespace c11httpd {


// HTTP header.
//
// <B>Example:</B>
// @code
// http_response response;
//
// response << http_header_t("Content-Type", "application/json;charset=UTF-8")
//          << http_header_t("Content-Encoding", "gzip");
// @endcode
class http_header_t {
public:
	// All header keys.
	//
	// This single instance defines all possible HTTP header keys.
	class all {
	public:
		static const all& instance() {
			return st_instance;
		}

		const std::vector<fast_str_t> request_all() const {
			return this->m_request;
		}

		const std::vector<fast_str_t> response_all() const {
			return this->m_response;
		}

		const fast_str_t* request_find(const fast_str_t& key) const;
		const fast_str_t* response_find(const fast_str_t& key) const;

	private:
		all();
		all(const all& another) = delete;
		all& operator=(const all& another) = delete;

	private:
		static all st_instance;

	private:
		std::vector<fast_str_t> m_request;
		std::vector<fast_str_t> m_response;
		fast_str_less_nocase_t m_less;
	};

public:
	static const fast_str_t Accept;
	static const fast_str_t Accept_Charset;
	static const fast_str_t Accept_Encoding;
	static const fast_str_t Accept_Language;

	// Accept-Ranges: bytes
	static const fast_str_t Accept_Ranges;
	static const fast_str_t App_Json_UTF8;
	static const fast_str_t App_Octet_Stream;
	static const fast_str_t App_RPM;
	static const fast_str_t App_XML_UTF8;
	static const fast_str_t App_ZIP;
	static const fast_str_t Attachment;
	static const fast_str_t Authorization;
	static const fast_str_t Bytes;
	static const fast_str_t Compress;
	static const fast_str_t Connection;
	static const fast_str_t Content_Encoding;

	// Content-Disposition: attachment; filename="file-name.dat"
	static const fast_str_t Content_Disposition;
	static const fast_str_t Content_Language;
	static const fast_str_t Content_Length;
	static const fast_str_t Content_Location;
	static const fast_str_t Content_MD5;
	static const fast_str_t Content_Range;
	static const fast_str_t Content_Type;
	static const fast_str_t Date;
	static const fast_str_t Deflate;
	static const fast_str_t Filename;
	static const fast_str_t GZIP;
	static const fast_str_t Host;
	static const fast_str_t HTTP_VERSION_1_1;
	static const fast_str_t Identify;
	static const fast_str_t Image_GIF;
	static const fast_str_t Image_JPEG;
	static const fast_str_t Image_PNG;
	static const fast_str_t Keep_Alive;
	static const fast_str_t Last_Modified;
	static const fast_str_t Location;
	static const fast_str_t Range;
	static const fast_str_t Server;
	static const fast_str_t Text_CSS_UTF8;
	static const fast_str_t Text_HTML_UTF8;
	static const fast_str_t Text_JavaScript;
	static const fast_str_t Text_Plain_UTF8;

public:
	http_header_t() = default;
	http_header_t(const fast_str_t& key, const fast_str_t& value)
		: m_key(key), m_value(value) {
	}

	fast_str_t& key() {
		return this->m_key;
	}

	const fast_str_t& key() const {
		return this->m_key;
	}

	void key(const fast_str_t& key) {
		this->m_key = key;
	}

	fast_str_t& value() {
		return this->m_value;
	}

	const fast_str_t& value() const {
		return this->m_value;
	}

	void value(const fast_str_t& value) {
		this->m_value = value;
	}

	bool operator<(const http_header_t& another) const {
		return m_key.cmpi(another.m_key) < 0;
	}


private:
	fast_str_t m_key;
	fast_str_t m_value;
};


template <typename T, typename Traits>
inline std::basic_ostream<T, Traits>& operator<<(
	std::basic_ostream<T, Traits>& ostream, const http_header_t& header) {
	return ostream << header.key() << ": " << header.value();
}


} // namespace c11httpd.

