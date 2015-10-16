/**
 * RESTFul API controller.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/http_method.h"
#include "c11httpd/http_request.h"
#include "c11httpd/http_response.h"
#include <string>
#include <functional>
#include <vector>
#include <tuple>


namespace c11httpd {


// RESTFul API controller.
class rest_controller_t {
public:
	typedef std::function<uint32_t(
		const http_request_t*,
		http_response_t*,
		const std::vector<std::string>& variables)
	> routine_t;

private:
	typedef std::tuple<std::string, http_method_t, routine_t,
			std::vector<std::string>, std::vector<std::string>> value_t;

public:
	rest_controller_t() = default;
	virtual ~rest_controller_t() = default;

	// Virtual host, e.g."www.vhost1.net".
	const std::string& virtual_host() {
		return this->m_virtual_host;
	}

	void virtual_host(const std::string& virtual_host) {
		this->m_virtual_host = virtual_host;
	}

	// URI root, e.g."/school/student".
	const std::string& uri_root() {
		return this->m_uri_root;
	}

	void uri_root(const std::string& uri_root) {
		this->m_uri_root = uri_root;
	}

	void add(const std::string& uri,
		http_method_t method,
		const routine_t& routine,
		const std::vector<std::string>& consumes = std::vector<std::string>(),
		const std::vector<std::string>& produces = std::vector<std::string>()
		) {
		this->m_routines.push_back(value_t(uri, method, routine, consumes, produces));
	}

	template <typename T>
	void add(const std::string& uri,
		http_method_t method,
		T* self,
		uint32_t (T::*mem_func)(const http_request_t*, http_response_t*, const std::vector<std::string>&),
		const std::vector<std::string>& consumes = std::vector<std::string>(),
		const std::vector<std::string>& produces = std::vector<std::string>()
		) {
		// TO-DO: Convert <self, mem_func> to std::function<>.
	}

private:
	std::string m_virtual_host;
	std::string m_uri_root;

	// Keep this container flat, we will create another
	// calculating module to dispatch request to each routine rapidly.
	std::vector<value_t> m_routines;
};


} // namespace c11httpd.

