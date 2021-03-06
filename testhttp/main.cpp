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

// My RESTFul API controller.
class my_ctrl_t : public c11httpd::rest_ctrl_t {
public:
	my_ctrl_t();

	c11httpd::rest_result_t handle_root(
			c11httpd::ctx_setter_t& ctx_setter,
			c11httpd::conn_session_t& session,
			const c11httpd::http_request_t& request,
			const std::vector<c11httpd::fast_str_t>& placeholders,
			c11httpd::http_response_t& response);
};

my_ctrl_t::my_ctrl_t() {
	this->add("/*", c11httpd::http_method_t::any, this,
		&my_ctrl_t::handle_root, "",
		c11httpd::http_header_t::App_Json_UTF8.to_str());
}

c11httpd::rest_result_t my_ctrl_t::handle_root(
		c11httpd::ctx_setter_t& ctx_setter,
		c11httpd::conn_session_t& session,
		const c11httpd::http_request_t& request,
		const std::vector<c11httpd::fast_str_t>& placeholders,
		c11httpd::http_response_t& response) {

	std::cout << "IP address: " << session << std::endl;
	std::cout << "Method: " << c11httpd::http_method_t::instance().to_str(request.method()) << std::endl;
	std::cout << "URI: " << request.uri() << std::endl;
	std::cout << "HTTP Version: " << request.http_version() << std::endl;
	std::cout << "Host Name: " << request.hostname() << std::endl;

	std::cout << "URI variables:" << std::endl;
	for (const auto& item : request.vars()) {
		std::cout << "\t" << item << std::endl;
	}

	std::cout << "Request Headers:" << std::endl;
	for (const auto& item : request.headers()) {
		std::cout << "\t" << item << std::endl;
	}

	std::cout << "Content:" << std::endl;
	std::cout << "{";
	std::cout.write(request.content(), request.content_length());
	std::cout << "}" << std::endl;

	std::cout << std::endl;

	response << c11httpd::http_header_t("HEADER-1", "header 1 value");
	response << "{\"hello\":\"world\",\"value\":true}";
	response.code(202);

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
//	acceptor.config().worker_processes(1);

	my_ctrl_t handler;
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


