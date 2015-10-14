/**
 * Process worker pool.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include <set>
#include "unistd.h"


namespace c11httpd {


// Process worker pool.
class process_pool_t {
public:
	process_pool_t() {
		this->m_child = false;
	}

	bool child_process() const {
		return this->m_child;
	}

	// Create a process worker.
	err_t create_worker(int number);

	// Kill a child process worker.
	//
	// If the current process is a child process, this function would do nothing.
	err_t kill_child(pid_t pid);

	// Kill all child process workers.
	//
	// If the current process is a child process, this function would do nothing.
	void kill_children();

	// Triggered when SIGCHLD is received.
	void on_terminated(pid_t pid);

private:
	std::set<pid_t> m_children;
	bool m_child;
};


} // namespace c11httpd.

