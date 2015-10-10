/**
 * c11httpd server daemon.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/all.h"
#include <iostream>


class my_event_handler : public c11httpd::conn_event_t {
public:
	virtual bool on_connected(c11httpd::conn_session_t* session);
	virtual void on_disconnected(c11httpd::conn_session_t* session);
	virtual void on_received(c11httpd::conn_session_t* session);
};

bool my_event_handler::on_connected(c11httpd::conn_session_t* session) {
	std::cout << *session << " was connected." << std::endl;
	return true;
}

void my_event_handler::on_disconnected(c11httpd::conn_session_t* session) {
	std::cout << *session << " was disconnected." << std::endl;
}

void my_event_handler::on_received(c11httpd::conn_session_t* session) {
	std::cout << *session << " -> ";

	auto& buf = session->recv_buf();
	auto size = buf.size();

	while (size > 0 && buf[size - 1] == '\n') {
		-- size;
		if (size > 0 && buf[size - 1] == '\r') {
			-- size;
		}
	}

	std::cout.write(buf.front(), size);
	std::cout << std::endl;
	buf.clear();
}


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

	my_event_handler handler;
	ret = acceptor.run(&handler);
	if (!ret) {
		std::cout << "acceptor::run() failed. " << ret << std::endl;
		return 1;
	}

	return 0;
}


