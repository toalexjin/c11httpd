/**
 * Process worker pool.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/worker_pool.h"
#include <sys/types.h>
#include <signal.h>


namespace c11httpd {


err_t worker_pool_t::create(int number) {
	if (!this->m_main_process) {
		return err_t();
	}

	for (int i = 0; i < number; ++i) {
		const auto pid = fork();
		if (pid == -1) {
			return err_t::current();
		}

		if (pid == 0) {
			this->m_main_process = false;
			break;
		} else {
			assert(this->m_main_process);
			this->m_workers.insert(pid);
		}
	}

	return err_t();
}

err_t worker_pool_t::kill(pid_t pid) {
	err_t ret;

	if (!this->m_main_process) {
		return ret;
	}

	auto pos = this->m_workers.find(pid);
	if (pos == this->m_workers.end()) {
		return ret;
	}

	if (::kill(pid, SIGTERM) == -1) {
		ret = err_t::current();
	}

	this->m_workers.erase(pos);
	return ret;
}

void worker_pool_t::kill_all() {
	if (!this->m_main_process) {
		return;
	}

	for (auto it = this->m_workers.cbegin(); it != this->m_workers.cend(); ++it) {
		::kill((*it), SIGTERM);
	}

	this->m_workers.clear();
}

bool worker_pool_t::on_terminated(pid_t pid) {
	if (!this->m_main_process) {
		return false;
	}

	return this->m_workers.erase(pid) > 0;
}


} // namespace c11httpd.

