/**
 * HTTP method.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_method.h"


namespace c11httpd {


// Single instance.
http_method_t http_method_t::st_instance;


http_method_t::http_method_t()
	: m_str_index({
		{"UNKNOWN", http_method_t::unknown},
		{"GET", http_method_t::get},
		{"POST", http_method_t::post},
		{"PUT", http_method_t::put},
		{"DELETE", http_method_t::del},
		{"OPTIONS", http_method_t::options},
		{"HEAD", http_method_t::head}
	}){

	for (const auto& item : m_str_index) {
		this->m_integer_index[item.second] = item.first;
	}
}

int http_method_t::to_integer(const fast_str_t& str) const {
	const auto it = m_str_index.find(str);
	return it == m_str_index.end() ? http_method_t::unknown : (*it).second;
}

const fast_str_t& http_method_t::to_str(int method) {
	if (method > http_method_t::unknown && method <= http_method_t::head) {
		return m_integer_index[method];
	} else {
		assert(false);
		return m_integer_index[http_method_t::unknown];
	}
}


} // namespace c11httpd.

