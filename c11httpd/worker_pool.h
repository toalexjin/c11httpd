/**
 * Worker process pool.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include <set>
#include <sys/types.h>
#include <unistd.h>


namespace c11httpd {


// Worker process pool.
class worker_pool_t {
public:
	worker_pool_t() {
		this->m_self_pid = getpid();
		this->m_main_process = true;
	}

	bool main_process() const {
		return this->m_main_process;
	}

	// Get process id.
	//
	// Note that in a signal handler function, Linux system API getpid()
	// would return the caller process that sends the Linux signal,
	// not the callee process receives the Linux signal.
	//
	// Different from Linux system API getpid(), self_pid() returns
	// the callee process id, not the caller process id.
	pid_t self_pid() const {
		return this->m_self_pid;
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
	pid_t m_self_pid;
	bool m_main_process;
};


} // namespace c11httpd.

