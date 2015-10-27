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


class my_context_t : public c11httpd::conn_ctx_t {
public:
	my_context_t() {
		this->clear();
	}

	virtual void clear();

	void set_default() {
		this->m_str = "Parse failed.";
		this->m_count = 1;
		this->m_index = 0;
	}

	void parse(char* first, const char* last);

	size_t index() const {
		return m_index;
	}

	size_t count() const {
		return m_count;
	}

	bool more() const {
		return m_index < m_count;
	}

	void next(c11httpd::buf_t& send_buf) {
		send_buf << "("
			<< std::to_string(this->m_index + 1)
			<< "@" << this->m_str << ")\r\n";

		this->m_index ++;
	}

private:
	std::string m_str;
	int m_count;
	int m_index;
};


void my_context_t::clear() {
	this->m_str.clear();
	this->m_count = 0;
	this->m_index = 0;
}

void my_context_t::parse(char* first, const char* last) {
	// Clear content.
	this->clear();

	// Skip starting whitespace.
	while (first != last && *first == ' ' && *first == '\t') {
		++first;
	}

	if (first == last) {
		this->set_default();
		return;
	}

	char* ptr = first + 1;
	while (ptr != last && *ptr != ' ' && *ptr != '\t') {
		++ptr;
	}

	if (ptr == last) {
		this->set_default();
		return;
	}

	*ptr = 0;
	if (std::sscanf(first, "%d", &m_count) != 1
		|| m_count <= 0) {
		this->set_default();
		return;
	}
	*ptr = ' ';

	// Skip whitespace.
	++ ptr;
	while (ptr != last && *ptr == ' ' && *ptr == '\t') {
		++ptr;
	}

	if (ptr == last) {
		this->set_default();
		return;
	}

	this->m_str.assign(ptr, last - ptr);
}


class my_event_handler_t : public c11httpd::conn_event_t {
public:
	virtual uint32_t on_connected(c11httpd::conn_session_t& session,
			c11httpd::buf_t& send_buf);

	virtual void on_disconnected(c11httpd::conn_session_t& session);

	virtual uint32_t on_received(c11httpd::conn_session_t& session,
			c11httpd::buf_t& recv_buf, c11httpd::buf_t& send_buf);

	virtual uint32_t get_more_data(c11httpd::conn_session_t& session,
			c11httpd::buf_t& send_buf);
};


uint32_t my_event_handler_t::on_connected(c11httpd::conn_session_t& session,
		c11httpd::buf_t& send_buf) {
	session.set_ctx(new my_context_t());

	std::cout << session << " was connected." << std::endl;

	send_buf << "=> hello, " << session.ip()
			<< ":" << std::to_string(session.port()) << "!\r\n";

	return 0;
}

void my_event_handler_t::on_disconnected(c11httpd::conn_session_t& session) {
	std::cout << session << " was disconnected." << std::endl;
}

uint32_t my_event_handler_t::on_received(c11httpd::conn_session_t& session,
		c11httpd::buf_t& recv_buf, c11httpd::buf_t& send_buf) {
	char* first = recv_buf.front();
	char* last = first;

	while (last != recv_buf.front() + recv_buf.size()) {
		if (*last == 0 || *last == '\r' || *last == '\n') {
			break;
		}

		++ last;
	}

	// Print at server side.
	std::cout << session << " -> (";
	std::cout.write(first, last - first);
	std::cout << ")" << std::endl;

	my_context_t* ctx = (my_context_t*) session.get_ctx();
	ctx->parse(first, last);

	// Clear recv data as we already processed it.
	recv_buf.clear();

	// Write data to send.
	ctx->next(send_buf);

	return ctx->more() ? c11httpd::event_result_more_data : 0;
}

uint32_t my_event_handler_t::get_more_data(c11httpd::conn_session_t& session,
		c11httpd::buf_t& send_buf) {
	my_context_t* ctx = (my_context_t*) session.get_ctx();
	ctx->next(send_buf);

	return ctx->more() ? c11httpd::event_result_more_data : 0;
}

static void help() {
	std::cout << "Usage: testtcp echo" << std::endl;
	std::cout << "       testtcp repeat" << std::endl;
	std::cout << std::endl;
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
	acceptor.worker_processes(1);

	if (std::strcmp(argv[1], "echo") == 0) {
		ret = acceptor.run_tcp([](
				c11httpd::conn_session_t& session,
				c11httpd::buf_t& recv_buf,
				c11httpd::buf_t& send_buf) -> uint32_t {
			std::cout << "{{";
			std::cout.write(recv_buf.front(), recv_buf.size());
			std::cout << "}}" << std::endl;
			send_buf << "[Echo] " << recv_buf;
			recv_buf.clear();
			return 0;
		});
	} else if (std::strcmp(argv[1], "repeat") == 0) {
		my_event_handler_t handler;
		ret = acceptor.run_tcp(&handler);
	} else {
		help();
		return 1;
	}

	if (acceptor.main_process()) {
		if (!ret) {
			std::cout << "acceptor::run() failed. " << ret << std::endl;
			return 1;
		}

		std::cout << "TCP Service exited gracefully." << std::endl;
	}

	return 0;
}


