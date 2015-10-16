# c11httpd

A Lightweight, Easy-To-Use, High-Performance httpd library in C++ C11.

There are several quite famous httpd projects like **Apache**, **Nginx** and
**Lighttpd**. They are in C, pure web server daemon, and offer high performance.
As a result, their plugin module interfaces are not friendly. If you want to
implement a C/C++ RESTFul service based on them, you need to take a long time
to study how to write a plugin module. Even though you know how to do it,
there are still two pending issues:
- There is no native MVC framework support. You have to parse URL,
  dispatch incoming requests to different GET/POST/PUT/DELETE routines
  that you implemented. There might be a 3-party MVC plugin library available
  for these httpd projects, but you have to study and it's not a all-in-one
  solution, which might make debug harder.
- If your program offers TCP service (or RESTFul service) as well as some other
  services, you could not use these httpd projects because they are
  not a library that could be seamlessly integrated into your program.

## Goals and Features

**c11httpd** is for developers, not for administrators.

- Enables you to create a TCP service (or RESTFul service) in your existing
  program quickly. Your program just needs to handle input & output,
  does not need to know anything about socket, ipv4 & ipv6, and does not
  require a 3-party web server to work, very clean.
- Supports over 10,000 concurrent connections because it leverages
  Linux epoll & aio (same as **Nginx**, **Lighttpd**).
- Native worker process pool support (I prefer process to thread because
  it's more robust). With it, you could create several worker processes ahead
  waiting for incoming requests. If any of them died, **c11httpd** would restart
  it automatically. Furthermore, you could bind each worker process to each
  CPU core to get a better performance.
- Offers a simple RESTFul MVC framework, enables you to easily dispatch
  incoming URL requests to different GET/POST/PUT/DELETE routines (`In Progress`).

## Design Concepts

- **Easy-To-Use**: c11httpd offers a very simple interface, you could create
  a TCP service (or RESTFul service) with only a few lines of code
  (see below **Examples** section).
- **High Performance**: c11httpd could support over 10,000 concurrent connections.
  Besides, although c11httpd is written in C++, it does not create C++ objects
  arbitrarily, which might bring performance issue. For instance, c11httpd uses
  two simple C-style doubly linked lists to save active & free connections. it's
  trying to achieve a balance between C++ OOP/Meta-programming & C data structure.
- **Simple Implementation**: Some C++ libraries (e.g. **boost**) are too crazy,
  use meta-programming (C++ Template) too much, although in some cases
  there is a better C-style solution. That's probably why **Linus Torvalds**
  dislikes C++ language. Meta-programming looks very cool (I used to like it
  very much :sleeping:), but to be honest, it might bring troubles:
   - Build failure error message is not easy-to-read.
   - Not easy to debug code with gdb.
   - Team members might have trouble to understand the code.

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
	acceptor.worker_processes(4);

	// Run TCP service.
	//
	// If Linux signal SIGINT or SIGTERM is recevied, the service will quit.
	acceptor.run_tcp([](
		c11httpd::conn_session_t* session,
		c11httpd::buf_t* recv_buf,
		c11httpd::buf_t* send_buf) -> uint32_t {

		// Add an echo prefix.
		*send_buf << "[Echo] " << *recv_buf;

		// We have processed this message, let's clear recv buffer
		// so that the data will not come back again.
		recv_buf->clear();

		return 0;
	});

	return 0;
}
```

### How to create a full life-cycle TCP service:

```C++
// My session context, created in "on_connected" event
// and released in "on_disconnected" event.
class my_ctx_t : public c11httpd::conn_ctx_t {
public:
	virtual void clear() {
		this->m_login_id.clear();
	}

	const std::string& login_id() const {
		return this->m_login_id;
	}

	void login_id(const std::string& login_id) {
		this->m_login_id = login_id;
	}

private:
	std::string m_login_id;
};

// My event handler.
class my_event_handler_t : public c11httpd::conn_event_t {
public:
	virtual uint32_t on_connected(
		c11httpd::conn_session_t* session,
		c11httpd::buf_t* send_buf) {

		// The session object might have a context object
		// that was used by previous connection.
		if (session->get_ctx() == 0) {
			session->set_ctx(new my_ctx_t());
		}

		// This context object is valid until "on_disconnected" event completes.
		auto ctx = (my_ctx_t*) session->get_ctx();

		// Use UTC time as login session id.
		//
		// It's not unique, to be enhanced.
		ctx->login_id(std::to_string(std::time(0));

		*send_buf << "Hello, " << session->ip() << ":"
			<< std::to_string(session->port()) << "\r\n";

		return 0;
	}

	virtual void on_disconnected(c11httpd::conn_session_t* session) {
		// There is no need to release the context object in this routine
		// because c11httpd library will do it after "on_disconnected" returns.
		auto ctx = (my_ctx_t*) session->get_ctx();

		std::cout << session->ip() << ":" << session->port()
			<< " was disconnected." << std::endl;
		std::cout << "Login id will be recycled:" << ctx->login_id() << std::endl;
	}

	virtual uint32_t on_received(
		c11httpd::conn_session_t* session,
		c11httpd::buf_t* recv_buf,
		c11httpd::buf_t* send_buf) {

		// Add an echo prefix.
		*send_buf << "[Echo] " << *recv_buf;

		// We have processed this message, let's clear recv buffer
		// so that the data will not come back again.
		recv_buf->clear();

		// Make "get_more_data" event be triggered.
		return c11httpd::event_result_more_data;
	}

	virtual uint32_t get_more_data(
		c11httpd::conn_session_t* session,
		c11httpd::buf_t* send_buf) {

		*send_buf << "Goodbye, " << session->ip() << ":"
			<< std::to_string(session->port()) << "\r\n";

		// Let's disconnect the connection.
		//
		// The client totally received three messages: hello, echo and goodbye.
		return c11httpd::event_result_disconnect;
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
	acceptor.worker_processes(4);

	// Run TCP service.
	//
	// If Linux signal SIGINT or SIGTERM is recevied, the service will quit.
	my_event_handler_t handler;
	acceptor.run_tcp(&handler);

	return 0;
}
```

### How to create a RESTFul service running on two virtual hosts (`In Progress`):

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
	acceptor.worker_processes(4);

	// This controller runs on virtual host "company.net".
	c11httpd::rest_controller_t c1("company.net",
		"/company", {}, {"application/json;charset=UTF-8"});

	// GET "/company/employee".
	c1.add("/employee", http_method_get, [](const http_request_t& request,
		http_response_t& response, const std::vector<std::string>& variables) -> uint32_t {

		// Should use a json parser to encode the string.
		response.content() << "[]";
		return 0;
	});

	// GET "/company/employee/?".
	c1.add("/employee/?", http_method_get, [](const http_request_t& request,
		http_response_t& response, const std::vector<std::string>& variables) -> uint32_t {

		// Should use a json parser to encode the string.
		response.content() << "{\"id\":\""
			<< variables[0] << "\",\"name\":\"Alex Jin\"}";
		return 0;
	});

	// This controller runs on virtual host "school.net".
	c11httpd::rest_controller_t c1("school.net",
		"/school", {}, {"application/json;charset=UTF-8"});

	// GET "/school/student".
	c1.add("/student", http_method_get, [](const http_request_t& request,
		http_response_t& response, const std::vector<std::string>& variables) -> uint32_t {

		// Should use a json parser to encode the string.
		response.content() << "[]";
		return 0;
	});

	// GET "/school/student/?".
	c1.add("/student/?", http_method_get, [](const http_request_t& request,
		http_response_t& response, const std::vector<std::string>& variables) -> uint32_t {

		// Should use a json parser to encode the string.
		response.content() << "{\"id\":\""
			<< variables[0] << "\",\"name\":\"Alex Jin\"}";
		return 0;
	});

	// Run TCP service.
	//
	// If Linux signal SIGINT or SIGTERM is recevied, the service will quit.
	acceptor.run_http({&c1, &c2});

	return 0;
}
```

