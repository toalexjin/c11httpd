# c11httpd

A lightweight httpd library in C++ C11.

## Design Concepts

- **Easy-To-Use**: There are lots of httpd open source projects. Some of them
  do not have an easy-to-use interface, you have to take lots of time to learn
  how to use them. c11httpd enables you to create a TCP/HTTP server service
  with just a few lines of code (see below **Examples** section).
- **High Performance**: c11httpd library leverages Linux asynchronous IO,
  could support over 10,000 concurrent connections. Although c11httpd is
  written in C++, it does not create objects arbitrarily, which might
  cause performance issue. For instance, c11httpd uses C-style
  doubly linked lists to save active & free connections. c11httpd is trying to
  get a good balance between C++ OOP/Meta-programming & C data structure.
- **Simple Implementation**: Some C++ libraries (e.g. **boost**) are too crazy,
  use meta-programming (C++ Template) too much, although in some cases
  there is a better C-style solution. That's probably why **Linus Torvalds**
  dislikes C++ language. Meta-programming looks very cool
  (I used to like it very much :sleeping:) but might bring troubles:
   - Build failure error message is not easy-to-read.
   - Not easy to debug code with gdb.
   - Team members might have trouble to read the code.

## Examples

- Below code shows how to create a TCP server service:

```C++
c11httpd::acceptor_t acceptor;

// Listen to TCP port 2000 (ipv4 & ipv6), 2001 (ipv4), 2002 (ipv6).
acceptor.bind({{"", 2000}, {"0.0.0.0", 2001}, {"::", 2002}});

// Create 4 worker processes. All of them listen to the same TCP ports
// and receive client incoming requests.
//
// The main process is pure management process,
// will restart worker processes if they diead.
acceptor.worker_processes(4);

// Run TCP service.
acceptor.run_tcp([](
	c11httpd::conn_session_t* session,
	c11httpd::buf_t* recv_buf,
	c11httpd::buf_t* send_buf) -> uint32_t {

	send_buf->push_back("[Echo From Server] ");
	send_buf->push_back(recv_buf->front(), recv_buf->size());
	recv_buf->clear();
	return 0;
});
```

- Below code shows how to create a HTTP web service:

```
// Pending
```

## Build Requirements

- The library supports Linux only because it uses some advanced Linux features.
- **Linux kernel x86_64 2.6.27** (or above).
- **g++ 4.8** (or above).

## Build

1. `make`: Output files go to **/exe** and **/obj**.
2. `make clean`: Remove all output files.

