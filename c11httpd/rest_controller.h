/**
 * RESTFul API controller.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn_session.h"
#include "c11httpd/ctx_setter.h"
#include "c11httpd/fast_str.h"
#include "c11httpd/http_method.h"
#include "c11httpd/http_request.h"
#include "c11httpd/http_response.h"
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>


namespace c11httpd {


namespace details {

// Unified routine interface for both C function & C++ member function.
template <typename Ret, typename P1, typename P2,
	typename P3, typename P4, typename P5>
class callable_t {
public:
	virtual ~callable_t() = default;
	virtual Ret invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
};

// Convert class member function to callable_t.
template <typename T, typename Ret, typename P1,
	typename P2, typename P3, typename P4, typename P5>
class callable_cpp_t : public callable_t<Ret, P1, P2, P3, P4, P5> {
	typedef Ret (T::*mem_function_t)(P1, P2, P3, P4, P5);

public:
	callable_cpp_t(T* self, mem_function_t mem_function)
		: m_self(self), m_mem_function(mem_function) {
	}
	virtual ~callable_cpp_t() = default;

	virtual Ret invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
		return m_self->*m_mem_function(p1, p2, p3, p4, p5);
	}

private:
	T* m_self;
	mem_function_t m_mem_function;
};

// Convert c function to callable_t.
template <typename Function, typename Ret, typename P1,
	typename P2, typename P3, typename P4, typename P5>
class callable_c_t : public callable_t<Ret, P1, P2, P3, P4, P5> {
public:
	callable_c_t(Function function)
		: m_function(function) {
	}
	virtual ~callable_c_t() = default;

	virtual Ret invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
		return m_function(p1, p2, p3, p4, p5);
	}

private:
	Function m_function;
};

} // namespace details.


// RESTFul API controller.
//
// YOu could use this controller class directly,
// or create a sub-class inherits from this class.
class rest_controller_t {
public:
	enum class result_t {
		// The request has been fully processed
		// and its result is saved to "response".
		done = 0,

		// The request has fatal error, no response
		// was generated and the connection
		// needs to be closed immediately.
		disconnect = 1
	};

public:
	// C routine prototype.
	typedef result_t (*routine_c_t)(
		ctx_setter_t&, // Context getter/setter.
		const conn_session_t&, // Connection session.
		const http_request_t&, // Input request.
		const std::vector<fast_str_t>&, // URI placeholder values.
		http_response_t& // Output response.
		);

	// C++ routine prototype.
	typedef std::function<
		result_t(
			ctx_setter_t&, // Context getter/setter.
			const conn_session_t&, // Connection session.
			const http_request_t&, // Input request.
			const std::vector<fast_str_t>&, // URI placeholder values.
			http_response_t& // Output response.
		)
	> routine_cpp_t;

	typedef details::callable_t<
		result_t,
		ctx_setter_t&, // Context getter/setter.
		const conn_session_t&, // Connection session.
		const http_request_t&, // Input request.
		const std::vector<fast_str_t>&, // URI placeholder values.
		http_response_t& // Output response.
	> routine_callable_t;

	typedef std::tuple<
		std::string, // URI. e.g. "/company/employee/?".
		http_method_t::type_t, // Method, e.g.GET/PUT/POST/DELETE.
		std::unique_ptr<routine_callable_t> // Routine.
	> api_t;

public:
	explicit rest_controller_t(
		const std::string& virtual_host = std::string(),
		const std::string& uri_root = std::string()
	) : m_virtual_host(virtual_host),
		m_uri_root(uri_root) {
	}

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

	const std::vector<api_t>& apis() const {
		return this->m_apis;
	}

	void add(const std::string& uri,
		http_method_t::type_t method,
		const routine_c_t& routine
		) {
		this->m_apis.push_back(
			api_t(
				uri,
				method,
				std::unique_ptr<routine_callable_t>(
						new details::callable_c_t<
							routine_c_t, result_t, ctx_setter_t&,
							const conn_session_t&, const http_request_t&,
							const std::vector<fast_str_t>&, http_response_t&
						>(routine)
				)
			)
		);
	}

	void add(const std::string& uri,
		http_method_t::type_t method,
		const routine_cpp_t& routine
		) {
		this->m_apis.push_back(
			api_t(
				uri,
				method,
				std::unique_ptr<routine_callable_t>(
						new details::callable_c_t<
							routine_cpp_t, result_t, ctx_setter_t&,
							const conn_session_t&, const http_request_t&,
							const std::vector<fast_str_t>&, http_response_t&
						>(routine)
				)
			)
		);
	}

	template <typename T>
	void add(const std::string& uri,
		http_method_t::type_t method,
		T* self,
		result_t (T::*routine)(ctx_setter_t&,
				const conn_session_t&, const http_request_t&,
				const std::vector<fast_str_t>&, http_response_t&)
		) {
		this->m_apis.push_back(
			api_t(
				uri,
				method,
				std::unique_ptr<routine_callable_t>(
					new details::callable_cpp_t<
						T, result_t, ctx_setter_t&,
						const conn_session_t&, const http_request_t&,
						const std::vector<fast_str_t>&, http_response_t&
					>(self, routine)
				)
			)
		);
	}

private:
	std::string m_virtual_host;
	std::string m_uri_root;

	// Keep this container flat, we will create another
	// calculating module to dispatch request to each routine quickly.
	std::vector<api_t> m_apis;
};


} // namespace c11httpd.

