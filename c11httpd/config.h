/**
 * TCP/HTTP Server Configuration.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {


// TCP/HTTP Server Configuration.
class config_t {
public:
	enum {
		// Support "Connection: keep-alive" or not.
		keep_alive = 1,

		// Support response "Date:???" or not.
		response_date = (1 << 1)
	};

public:
	config_t();
	config_t(const config_t&) = default;
	config_t& operator=(const config_t&) = default;

	bool enabled(uint32_t flag) const {
		return (m_flags & flag) != 0;
	}

	void enable(uint32_t flag) {
		m_flags |= flag;
	}

	void disable(uint32_t flag) {
		m_flags &= ~flag;
	}

	// Get number of worker processes.
	int worker_processes() const {
		return this->m_worker_processes;
	}

	// Set number of worker processes.
	//
	// -# If the number is zero, then the main process does everything,
	//    including receiving incoming client requests.
	// -# If the number is greater than zero, than the main process
	//    would become a pure management process, will NOT receive
	//    incoming client requests and will restart worker processes if they died.
	void worker_processes(int worker_processes) {
		assert(worker_processes >= 0);
		this->m_worker_processes = worker_processes;
	}

	int backlog() const {
		return this->m_backlog;
	}

	void backlog(int value) {
		if (value > 0) {
			this->m_backlog = value;
		}
	}

	int max_epoll_events() const {
		return this->m_max_epoll_events;
	}

	void max_epoll_events(int value) {
		if (value > 0) {
			this->m_max_epoll_events = value;
		}
	}

	int max_free_connection() const {
		return this->m_max_free_connection;
	}

	void max_free_connection(int value) {
		if (value >= 0) {
			this->m_max_free_connection = value;
		}
	}

	// Set all to default values.
	void set_default();

private:
	uint32_t m_flags;
	int m_worker_processes;
	int m_backlog;
	int m_max_epoll_events;
	int m_max_free_connection;
};


} // namespace c11httpd.

