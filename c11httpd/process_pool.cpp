/**
 * Process worker pool.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/process_pool.h"
#include <sys/types.h>
#include <signal.h>


namespace c11httpd {


// Create a process worker.
err_t process_pool_t::create_worker(int number) {
	for (int i = 0; i < number; ++i) {
		const auto pid = fork();
		if (pid == -1) {
			return err_t::current();
		}

		if (pid == 0) {
			this->m_child = true;
			break;
		} else {
			assert(!this->m_child);
			this->m_children.insert(pid);
		}
	}

	return err_t();
}

err_t process_pool_t::kill_child(pid_t pid) {
	err_t ret;

	if (this->m_child) {
		return ret;
	}

	auto pos = this->m_children.find(pid);
	if (pos == this->m_children.end()) {
		return ret;
	}

	if (kill(pid, SIGTERM) == -1) {
		ret = err_t::current();
	}

	this->m_children.erase(pos);
	return ret;
}

void process_pool_t::kill_children() {
	if (this->m_child) {
		return;
	}

	for (auto it = this->m_children.cbegin(); it != this->m_children.cend(); ++it) {
		kill((*it), SIGTERM);
	}

	this->m_children.clear();
}

void process_pool_t::on_terminated(pid_t pid) {
	this->m_children.erase(pid);
}


} // namespace c11httpd.

