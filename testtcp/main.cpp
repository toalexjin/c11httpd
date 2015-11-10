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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


class my_context_t : public c11httpd::ctx_t {
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
	virtual uint32_t on_connected(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		c11httpd::conn_session_t& session,
		c11httpd::buf_t& send_buf);

	virtual void on_disconnected(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		c11httpd::conn_session_t& session);

	virtual uint32_t on_received(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		c11httpd::conn_session_t& session,
		c11httpd::buf_t& recv_buf, c11httpd::buf_t& send_buf);

	virtual uint32_t get_more_data(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		c11httpd::conn_session_t& session,
		c11httpd::buf_t& send_buf);

	virtual uint32_t on_aio_completed(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		c11httpd::conn_session_t& session,
		const std::vector<c11httpd::aio_t>& completed,
		c11httpd::buf_t& send_buf);
};


uint32_t my_event_handler_t::on_connected(
	c11httpd::ctx_setter_t& ctx_setter,
	const c11httpd::config_t& cfg,
	c11httpd::conn_session_t& session,
	c11httpd::buf_t& send_buf) {
	ctx_setter.ctx(new my_context_t());

	std::cout << session << " was connected." << std::endl;

	send_buf << "=> hello, " << session.ip()
			<< ":" << std::to_string(session.port()) << "!\r\n";

	return 0;
}

void my_event_handler_t::on_disconnected(
	c11httpd::ctx_setter_t& ctx_setter,
	const c11httpd::config_t& cfg,
	c11httpd::conn_session_t& session) {
	std::cout << session << " was disconnected." << std::endl;
}

uint32_t my_event_handler_t::on_received(
	c11httpd::ctx_setter_t& ctx_setter,
	const c11httpd::config_t& cfg,
	c11httpd::conn_session_t& session,
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

	my_context_t* ctx = (my_context_t*) ctx_setter.ctx();
	ctx->parse(first, last);

	// Clear recv data as we already processed it.
	recv_buf.clear();

	// Write data to send.
	ctx->next(send_buf);

	return ctx->more() ? c11httpd::conn_event_t::result_more_data : 0;
}

uint32_t my_event_handler_t::get_more_data(
	c11httpd::ctx_setter_t& ctx_setter,
	const c11httpd::config_t& cfg,
	c11httpd::conn_session_t& session,
	c11httpd::buf_t& send_buf) {

	my_context_t* ctx = (my_context_t*) ctx_setter.ctx();
	ctx->next(send_buf);

	return ctx->more() ? c11httpd::conn_event_t::result_more_data : 0;
}

uint32_t my_event_handler_t::on_aio_completed(
	c11httpd::ctx_setter_t& ctx_setter,
	const c11httpd::config_t& cfg,
	c11httpd::conn_session_t& session,
	const std::vector<c11httpd::aio_t>& completed,
	c11httpd::buf_t& send_buf) {

	return 0;
}

class file_ctx_t : public c11httpd::ctx_t {
public:
	virtual void clear();

	c11httpd::fd_t m_fd;
};

void file_ctx_t::clear() {
	m_fd.close();
}

static void help() {
	std::cout << "Usage: testtcp echo" << std::endl;
	std::cout << "       testtcp repeat" << std::endl;
	std::cout << "       testtcp file" << std::endl;
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
	acceptor.config().worker_processes(0);

	if (std::strcmp(argv[1], "echo") == 0) {
		ret = acceptor.run_tcp([](
				c11httpd::ctx_setter_t& ctx_setter,
				const c11httpd::config_t& cfg,
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
	} else if (std::strcmp(argv[1], "file") == 0) {
		c11httpd::conn_event_adapter_t adapter;

		// on_received().
		adapter.lambda_on_received([](
				c11httpd::ctx_setter_t& ctx_setter,
				const c11httpd::config_t& cfg,
				c11httpd::conn_session_t& session,
				c11httpd::buf_t& recv_buf,
				c11httpd::buf_t& send_buf) -> uint32_t {

			if (ctx_setter.ctx() == 0) {
				ctx_setter.ctx(new file_ctx_t());
			}

			file_ctx_t* ctx = (file_ctx_t*) ctx_setter.ctx();
			if (ctx->m_fd.is_open()) {
				return 0;
			}

			c11httpd::fast_str_t str(recv_buf.front(), recv_buf.size());
			c11httpd::fast_str_t line;
			if (!str.getline(&line) || line.empty()) {
				std::cout << "Could not get a line." << std::endl;
				return c11httpd::conn_event_t::result_disconnect;
			}

			std::cout << "Open file " << line << std::endl;

			struct stat info;
			if (stat(line.c_str(), &info) != 0 || !S_ISREG(info.st_mode)) {
				std::cout << "It is not a valid file." << std::endl;
				return c11httpd::conn_event_t::result_disconnect;
			}

			ctx->m_fd = ::open(line.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK);
			if (!ctx->m_fd.is_open()) {
				std::cout << "Could not open file." << std::endl;
				return c11httpd::conn_event_t::result_disconnect;
			}

			auto ret = session.aio_read(
				ctx->m_fd, 0, send_buf.back(info.st_size), info.st_size);

			if (!ret) {
				std::cout << "aio_read() failed: " << ret << std::endl;
				return c11httpd::conn_event_t::result_disconnect;
			}

			std::cout << "aio_read() ok." << std::endl;

			recv_buf.clear();
			return 0;
		});

		// on_aio_completed().
		adapter.lambda_on_aio_completed([](
				c11httpd::ctx_setter_t& ctx_setter,
				const c11httpd::config_t& cfg,
				c11httpd::conn_session_t& session,
				const std::vector<c11httpd::aio_t>& completed,
				c11httpd::buf_t& send_buf) -> uint32_t {

			file_ctx_t* ctx = (file_ctx_t*) ctx_setter.ctx();
			ctx->clear();

			if (completed.empty()) {
				assert(false);
				return c11httpd::conn_event_t::result_disconnect;
			}

			if (completed[0].m_error.failed()) {
				std::cout << "aio_completed: " << completed[0].m_error << std::endl;
				return c11httpd::conn_event_t::result_disconnect;
			}

			send_buf.size(send_buf.size() + completed[0].m_ok_bytes);
			return 0;
		});

		ret = acceptor.run_tcp(&adapter);
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


