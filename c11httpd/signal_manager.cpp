/**
 * Signal manager.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/signal_manager.h"
#include <signal.h>
#include <mutex>
#include <errno.h>
#include <set>
#include <cstring>


namespace {

// Lock.
std::recursive_mutex st_lock;

// Max signal number.
constexpr int st_max_signal = 64;

// Signal handlers.
//
// We allow to register multiple handlers for a single signal.
std::set<c11httpd::signal_event_t*> st_handlers[st_max_signal];

// Old signal handlers (used to restore).
struct sigaction st_old[st_max_signal];


void signal_proc(int signum) {
	if (signum <= 0 || signum > st_max_signal) {
		return;
	}

	// Save errno.
	const auto old = errno;

	// Lock.
	st_lock.lock();

	// Call each function registered to the same signal.
	for (auto it = st_handlers[signum - 1].cbegin();
			it != st_handlers[signum - 1].cend();
			++it) {
		(*it)->on_signal(signum);
	}

	// Unlock
	st_lock.unlock();

	// Restore errno.
	errno = old;
}


void add_signal_hooker(int signum) {
	struct sigaction sa;

	bzero(&sa, sizeof(sa));
	sa.sa_handler = &signal_proc;
	sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);
	sigaction(signum, &sa, &st_old[signum - 1]);
}


void remove_signal_hooker(int signum) {
	sigaction(signum, &st_old[signum - 1], 0);
}


} // unnamed namespace.


namespace c11httpd {

signal_manager_t signal_manager_t::st_instance;


err_t signal_manager_t::add(const std::vector<int>& signals, signal_event_t* handler) {
	assert(st_max_signal == SIGRTMAX);

	// Check if any signal number is not in range [1,st_max_signal].
	for (auto it = signals.cbegin(); it != signals.cend(); ++it) {
		if (*it <= 0 || *it > st_max_signal) {
			assert(false);
			return EINVAL;
		}
	}

	// Lock.
	st_lock.lock();

	for (auto it = signals.cbegin(); it != signals.cend(); ++it) {
		if (st_handlers[(*it) - 1].empty()) {
			add_signal_hooker(*it);
		}
		st_handlers[(*it) - 1].insert(handler);
	}

	// Unlock
	st_lock.unlock();

	return err_t();
}

void signal_manager_t::remove(const std::vector<int>& signals, signal_event_t* handler) {
	// Lock.
	st_lock.lock();

	for (auto it = signals.cbegin(); it != signals.cend(); ++it) {
		st_handlers[(*it) - 1].erase(handler);
		if (st_handlers[(*it) - 1].empty()) {
			remove_signal_hooker(*it);
		}
	}

	// Unlock
	st_lock.unlock();
}

void signal_manager_t::ignore(int signum) {
	signal(signum, SIG_IGN);
}



} // namespace c11httpd.

