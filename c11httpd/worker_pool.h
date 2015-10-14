/**
 * Worker process pool.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include <set>
#include "unistd.h"


namespace c11httpd {


// Worker process pool.
class worker_pool_t {
public:
	worker_pool_t() {
		this->m_main_process = true;
	}

	bool main_process() const {
		return this->m_main_process;
	}

	// Create a process worker.
	err_t create(int number);

	// Kill a single worker process.
	//
	// If the current process is not main process, this function would do nothing.
	err_t kill(pid_t pid);

	// Kill all worker processes.
	//
	// If the current process is not main process, this function would do nothing.
	void kill_all();

	// Triggered when SIGCHLD is received.
	bool on_terminated(pid_t pid);

private:
	std::set<pid_t> m_workers;
	bool m_main_process;
};


} // namespace c11httpd.

