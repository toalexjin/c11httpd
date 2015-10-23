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
	: m_types({
		{"GET", http_method_t::get},
		{"POST", http_method_t::post},
		{"PUT", http_method_t::put},
		{"DELETE", http_method_t::del},
		{"OPTIONS", http_method_t::options},
		{"HEAD", http_method_t::head}
	}){
}


} // namespace c11httpd.

