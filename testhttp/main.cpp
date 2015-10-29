/**
 * c11httpd server daemon.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/all.h"
#include <iostream>
#include <cstring>
#include <string>
#include <signal.h>


static void help() {
	std::cout << "Usage: testhttp rest" << std::endl;
	std::cout << std::endl;
}

class my_controller_t : public c11httpd::rest_controller_t {
public:
	my_controller_t();

	c11httpd::rest_result_t handle_root(
			c11httpd::ctx_setter_t& ctx_setter,
			const c11httpd::conn_session_t& session,
			const c11httpd::http_request_t& request,
			const std::vector<c11httpd::fast_str_t>& placeholders,
			c11httpd::http_response_t& response);
};

my_controller_t::my_controller_t() {
	this->add("/*", c11httpd::http_method_t::get, this, &my_controller_t::handle_root);
	this->add("/*", c11httpd::http_method_t::post, this, &my_controller_t::handle_root);
	this->add("/*", c11httpd::http_method_t::put, this, &my_controller_t::handle_root);
	this->add("/*", c11httpd::http_method_t::del, this, &my_controller_t::handle_root);
}

c11httpd::rest_result_t my_controller_t::handle_root(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::conn_session_t& session,
		const c11httpd::http_request_t& request,
		const std::vector<c11httpd::fast_str_t>& placeholders,
		c11httpd::http_response_t& response) {

	std::cout << "IP address: " << session << std::endl;
	std::cout << "Method: " << c11httpd::http_method_t::instance().to_str(request.method()) << std::endl;
	std::cout << "URI: " << request.uri() << std::endl;
	std::cout << "HTTP Version: " << request.http_version() << std::endl;
	std::cout << "Host: " << request.host() << std::endl;
	std::cout << "Request Headers:" << std::endl;
	for (const auto& item : request.headers()) {
		std::cout << "\t" << item << std::endl;
	}

	std::cout << "Content:" << std::endl;
	std::cout << "{";
	std::cout.write(request.content(), request.content_length());
	std::cout << "}" << std::endl;

	std::cout << std::endl;

	response.code(201);
	response << c11httpd::http_header_t("HEADER-1", "header 1 value");
	response << c11httpd::http_header_t("Content-Type", "application/json;charset=UTF-8");
	response << "{\"hello\":\"world\",\"value\":true}";

	return c11httpd::rest_result_t::done;
}


int main(int argc, char* argv[]) {

	if (argc != 2) {
		help();
		return 1;
	}

	c11httpd::err_t ret;
	c11httpd::acceptor_t acceptor;

	ret = acceptor.bind({{"", 2000}, {"0.0.0.0", 2001}, {"::", 2002}});
	if (!ret) {
		std::cout << "acceptor::bind() failed. " << ret << std::endl;
		return 1;
	}

	const auto binds(acceptor.binds());
	for (auto it = binds.cbegin(); it != binds.cend(); ++it) {
		std::cout << "Listen-> " << (*it).first << ":" << (*it).second << std::endl;
	}

	// Totally three worker processes.
//	acceptor.worker_processes(1);

	my_controller_t handler;
	acceptor.run_http(&handler);

	if (acceptor.main_process()) {
		if (!ret) {
			std::cout << "acceptor::run() failed. " << ret << std::endl;
			return 1;
		}

		std::cout << "HTTP Service exited gracefully." << std::endl;
	}

	return 0;
}


