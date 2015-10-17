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
#include <memory>


namespace c11httpd {


namespace details {

// Unified routine interface for both C function & C++ member function.
template <typename Ret, typename P1, typename P2, typename P3>
class callable_t {
public:
	virtual ~callable_t() = default;
	virtual Ret invoke(P1 p1, P2 p2, P3 p3) = 0;
};

// Convert class member function to callable_t.
template <typename T, typename Ret, typename P1, typename P2, typename P3>
class callable_cpp_t : public callable_t<Ret, P1, P2, P3> {
	typedef Ret (T::*mem_function_t)(P1, P2, P3);

public:
	callable_cpp_t(T* self, mem_function_t mem_function)
		: m_self(self), m_mem_function(mem_function) {
	}
	virtual ~callable_cpp_t() = default;

	virtual Ret invoke(P1 p1, P2 p2, P3 p3) {
		return m_self->*m_mem_function(p1, p2, p3);
	}

private:
	T* m_self;
	mem_function_t m_mem_function;
};

// Convert c function to callable_t.
template <typename Function, typename Ret, typename P1, typename P2, typename P3>
class callable_c_t : public callable_t<Ret, P1, P2, P3> {
public:
	callable_c_t(Function function)
		: m_function(function) {
	}
	virtual ~callable_c_t() = default;

	virtual Ret invoke(P1 p1, P2 p2, P3 p3) {
		return m_function(p1, p2, p3);
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
	// C routine prototype.
	typedef uint32_t (*routine_c_t)(
		const http_request_t&, http_response_t&,
		const std::vector<std::string>&);

	// C++ routine prototype.
	typedef std::function<
		uint32_t(const http_request_t&,
		http_response_t&,
		const std::vector<std::string>&)
	> routine_cpp_t;

private:
	typedef details::callable_t<
		uint32_t,
		const http_request_t&,
		http_response_t&,
		const std::vector<std::string>&
	> routine_callable_t;

	typedef std::tuple<
		std::string,
		http_method_t,
		std::unique_ptr<routine_callable_t>,
		std::vector<std::string>,
		std::vector<std::string>
	> value_t;

public:
	explicit rest_controller_t(
		const std::string& virtual_host = std::string(),
		const std::string& uri_root = std::string(),
		const std::vector<std::string>& consumes = std::vector<std::string>(),
		const std::vector<std::string>& produces = std::vector<std::string>()
	) : m_virtual_host(virtual_host),
		m_uri_root(uri_root),
		m_consumes(consumes),
		m_produces(produces) {
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

	void add(const std::string& uri,
		http_method_t method,
		const routine_c_t& routine,
		const std::vector<std::string>& consumes = std::vector<std::string>(),
		const std::vector<std::string>& produces = std::vector<std::string>()
		) {
		this->m_routines.push_back(value_t(uri, method,
			std::unique_ptr<routine_callable_t>(
				new details::callable_c_t<
					routine_c_t, uint32_t, const http_request_t&,
					http_response_t&, const std::vector<std::string>&
				>(routine)
			), consumes, produces));
	}

	void add(const std::string& uri,
		http_method_t method,
		const routine_cpp_t& routine,
		const std::vector<std::string>& consumes = std::vector<std::string>(),
		const std::vector<std::string>& produces = std::vector<std::string>()
		) {
		this->m_routines.push_back(value_t(uri, method,
			std::unique_ptr<routine_callable_t>(
				new details::callable_c_t<
					routine_cpp_t, uint32_t, const http_request_t&,
					http_response_t&, const std::vector<std::string>&
				>(routine)
			), consumes, produces));
	}

	template <typename T>
	void add(const std::string& uri,
		http_method_t method,
		T* self,
		uint32_t (T::*routine)(const http_request_t&, http_response_t&, const std::vector<std::string>&),
		const std::vector<std::string>& consumes = std::vector<std::string>(),
		const std::vector<std::string>& produces = std::vector<std::string>()
		) {
		this->m_routines.push_back(value_t(uri, method,
			std::unique_ptr<routine_callable_t>(
				new details::callable_cpp_t<
					T, uint32_t, const http_request_t&,
					http_response_t&, const std::vector<std::string>&
				>(self, routine)
			), consumes, produces));
	}

private:
	std::string m_virtual_host;
	std::string m_uri_root;
	std::vector<std::string> m_consumes;
	std::vector<std::string> m_produces;

	// Keep this container flat, we will create another
	// calculating module to dispatch request to each routine rapidly.
	std::vector<value_t> m_routines;
};


} // namespace c11httpd.

