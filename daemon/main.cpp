/**
 * c11httpd server daemon.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/all.h"
#include <iostream>


int main(int argc, char* argv[]) {

	c11httpd::err_t ret;
	c11httpd::acceptor_t acceptor;

	ret = acceptor.bind({{"", 2000}, {"0.0.0.0", 2001}, {"::", 2002}});
	if (!ret) {
		std::cout << "acceptor::bind() failed. " << ret << std::endl;
		return 1;
	}

	const auto binds(acceptor.binds());
	for (auto it = binds.cbegin(); it != binds.end(); ++it) {
		std::cout << "Listen-> " << (*it).first << ":" << (*it).second << std::endl;
	}

	ret = acceptor.run();
	if (!ret) {
		std::cout << "acceptor::run() failed. " << ret << std::endl;
		return 1;
	}

	return 0;
}


