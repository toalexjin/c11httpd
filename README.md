# c11httpd

A lightweight, easy-to-use, high-performance httpd library in C++ C11.

There are several quite famous httpd projects like **Apache**, **Nginx** and
**Lighttpd**. They are in C, pure web server daemon, and offer high performance.
As a result, their plugin module interfaces are not friendly. If you want to
implement a C/C++ RESTFul service based on them, you need to take a long time
to study how to write a plugin module. Even though you know how to do it,
there are still two pending issues:
- If your program offers TCP service (or RESTFul service) as well as some
  other services, you could not use these httpd projects because they are
  not a library that could be seamlessly integrated into your program.
- There is no native RESTFul framework support. You have to parse URI,
  dispatch incoming requests to different GET/POST/PUT/DELETE routines
  that you implemented. There might be a 3-party RESTFul plugin library available
  for these httpd projects, but you have to study and it's not a all-in-one
  solution, which might make debug harder.

## Features and Design Concepts

**c11httpd** is for developers, not for administrators.

- **Easy-To-Use**: c11httpd offers a very simple interface, you could create
  a TCP service (or RESTFul service) with only a few lines of code
  (see below **Examples** section). Your program does not need to know anything
  about socket, ipv4 & ipv6, and does not require a 3-party web server to work.
  The entire solution is very clean.
- **High Performance**: c11httpd supports over 10,000 concurrent connections
  as it leverages Linux epoll & aio technologies (same as **Nginx**, **Lighttpd**).
  Although the library is written in C++, it does not create C++ objects
  arbitrarily, which might bring performance issue. For instance, c11httpd uses
  two simple C-style doubly linked lists to save active & free connections. it's
  trying to achieve a balance between C++ OOP/Meta-programming & C data structure.
- **Worker Process Pool**: c11httpd has native worker process pool support
  (I prefer process to thread because it's more robust). With it, you could
  create several worker processes ahead waiting for incoming requests. If any
  of them died, the library would restart it automatically. Furthermore,
  you could bind each worker process to each CPU core to get a better performance.
- **RESTFul Framework**: c11httpd offers a simple RESTFul framework, you could
  easily dispatch incoming requests to different GET/POST/PUT/DELETE
  routines (`In Progress`).
- **Virtual Host**: c11httpd enables you to create several virtual hosts
  running on the same TCP ports (`In Progress`).
- **Simple Implementation**: c11httpd's interface and implementation are simple,
  use meta-programming (C++ Template) but does not widely use it. This is
  different from most of modern C++ libraries (e.g. **boost**), which are
  almost pure C++ Template Libraries. C++ Template is very cool (I used to be a
  fan of it :sleeping:), but to be honest, it might bring following troubles
  (Btw, Probably because of C++ Template, **Linus Torvalds** dislikes C++):
   - Build failure error message is not easy-to-read.
   - Not easy to debug code with gdb.
   - Team members and library users might have trouble to understand the code,
     bug fix, tech support, ...

## Build Requirements

- The library supports Linux only because it uses some advanced Linux features.
- **Linux kernel x86_64 2.6.27** (or above).
- **g++ 4.8** (or above).

## Build

1. `make`: Generate **/obj/c11httpd.a**, **/exe/testtcp**, **/exe/testhttp**.
2. `make clean`: Remove all output files.

## Examples

### How to create a simple echo TCP service:

```C++
int main() {
	c11httpd::acceptor_t acceptor;

	// Listen to TCP port 2000 (ipv4 & ipv6), 2001 (ipv4), 2002 (ipv6).
	acceptor.bind({{"", 2000}, {"0.0.0.0", 2001}, {"::", 2002}});

	// Create 4 worker processes. All of them listen to
	// the same TCP ports and receive client incoming requests.
	//
	// The main process is pure management process,
	// will restart worker processes if they died.
	acceptor.config().worker_processes(4);

	// Run TCP service.
	//
	// If Linux signal SIGINT or SIGTERM is recevied, the service will quit.
	acceptor.run_tcp([](
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		const c11httpd::conn_session_t& session,
		c11httpd::buf_t& recv_buf,
		c11httpd::buf_t& send_buf) -> uint32_t {

		// Add an echo prefix.
		send_buf << "[Echo] " << recv_buf;

		// We have processed this message, let's clear recv buffer
		// so that the data will not come back again.
		recv_buf.clear();

		return 0;
	});

	return 0;
}
```

### How to create a full life-cycle TCP service:

```C++
// My event handler.
class my_event_handler_t : public c11httpd::conn_event_t {
public:
	virtual uint32_t on_connected(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		const c11httpd::conn_session_t& session,
		c11httpd::buf_t& send_buf) {
		send_buf << "Hello, " << session.ip() << ":"
			<< std::to_string(session.port()) << "\r\n";

		return 0;
	}

	virtual void on_disconnected(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		const c11httpd::conn_session_t& session) {
		std::cout << session.ip() << ":" << session.port()
			<< " was disconnected." << std::endl;
	}

	virtual uint32_t on_received(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		const c11httpd::conn_session_t& session,
		c11httpd::buf_t& recv_buf,
		c11httpd::buf_t& send_buf) {

		// Add an echo prefix.
		send_buf << "[Echo] " << recv_buf;

		// We have processed this message, let's clear recv buffer
		// so that the data will not come back again.
		recv_buf.clear();

		// Make "get_more_data" event be triggered.
		return c11httpd::conn_event_t::result_more_data;
	}

	virtual uint32_t get_more_data(
		c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::config_t& cfg,
		const c11httpd::conn_session_t& session,
		c11httpd::buf_t& send_buf) {

		send_buf << "Goodbye, " << session.ip() << ":"
			<< std::to_string(session.port()) << "\r\n";

		// Let's disconnect the connection.
		//
		// The client totally received three messages: hello, echo and goodbye.
		return c11httpd::conn_event_t::result_disconnect;
	}
};

int main() {
	c11httpd::acceptor_t acceptor;

	// Listen to TCP port 2000 (ipv4 & ipv6), 2001 (ipv4), 2002 (ipv6).
	acceptor.bind({{"", 2000}, {"0.0.0.0", 2001}, {"::", 2002}});

	// Create 4 worker processes. All of them listen to
	// the same TCP ports and receive client incoming requests.
	//
	// The main process is pure management process,
	// will restart worker processes if they died.
	acceptor.config().worker_processes(4);

	// Run TCP service.
	//
	// If Linux signal SIGINT or SIGTERM is recevied, the service will quit.
	my_event_handler_t handler;
	acceptor.run_tcp(&handler);

	return 0;
}
```

### How to create a RESTFul service running on two virtual hosts:

```C++
int main() {
	c11httpd::acceptor_t acceptor;

	// Listen to TCP port 2000 (ipv4 & ipv6), 2001 (ipv4), 2002 (ipv6).
	acceptor.bind({{"", 2000}, {"0.0.0.0", 2001}, {"::", 2002}});

	// Create 4 worker processes. All of them listen to
	// the same TCP ports and receive client incoming requests.
	//
	// The main process is pure management process,
	// will restart worker processes if they died.
	acceptor.config().worker_processes(4);

	// Controller for "company.net".
	c11httpd::rest_ctrl_t c1("company.net", "/company");

	// GET "/company/employee".
	c1.add("/employee", c11httpd::http_method_t::get,
		[](c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::conn_session_t& session,
		const c11httpd::http_request_t& request,
		const std::vector<c11httpd::fast_str_t>& placeholders
		c11httpd::http_response_t& response
		) -> c11httpd::rest_result_t {

		// Set HTTP response headers.
		response << c11httpd::http_header_t("Content-Type", "application/json;charset=UTF-8");

		// Write response content.
		// Note that the user should use a json parser to encode the string.
		response << "[]";

		return c11httpd::rest_result_t::done;
	});

	// GET "/company/employee/?".
	c1.add("/employee/?", c11httpd::http_method_t::get,
		[](c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::conn_session_t& session,
		const c11httpd::http_request_t& request,
		const std::vector<c11httpd::fast_str_t>& placeholders
		c11httpd::http_response_t& response
		) -> c11httpd::rest_result_t {

		// Set HTTP response headers.
		response << http_header_t("Content-Type", "application/json;charset=UTF-8");

		// Write response content.
		// Note that the user should use a json parser to encode the string.
		response << "{\"id\":\"" << variables[0] << "\",\"name\":\"Alex Jin\"}";

		return c11httpd::rest_result_t::done;
	});

	// Controller for "school.net".
	c11httpd::rest_ctrl_t c2("school.net", "/school");

	// GET "/school/student".
	c2.add("/student", c11httpd::http_method_t::get,
		[](c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::conn_session_t& session,
		const c11httpd::http_request_t& request,
		const std::vector<c11httpd::fast_str_t>& placeholders
		c11httpd::http_response_t& response
		) -> c11httpd::rest_result_t {

		// Set HTTP response headers.
		response << http_header_t("Content-Type", "application/json;charset=UTF-8");

		// Write response content.
		// Note that the user should use a json parser to encode the string.
		response << "[]";

		return c11httpd::rest_result_t::done;
	});

	// GET "/school/student/?".
	c2.add("/student/?", c11httpd::http_method_t::get,
		[](c11httpd::ctx_setter_t& ctx_setter,
		const c11httpd::conn_session_t& session,
		const c11httpd::http_request_t& request,
		const std::vector<c11httpd::fast_str_t>& placeholders
		c11httpd::http_response_t& response
		) -> c11httpd::rest_result_t {

		// Set HTTP response headers.
		response << c11httpd::http_header_t("Content-Type", "application/json;charset=UTF-8");

		// Not found.
		response.code(c11httpd::http_status_t::not_found);

		return c11httpd::rest_result_t::done;
	});

	// Run RESTFul service.
	//
	// If Linux signal SIGINT or SIGTERM is recevied, the service will quit.
	acceptor.run_http({&c1, &c2});

	return 0;
}
```

