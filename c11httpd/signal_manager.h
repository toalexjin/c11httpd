/**
 * Signal manager.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/err.h"
#include "c11httpd/signal_event.h"
#include <vector>


namespace c11httpd {


// Signal manager.
//
// Allow user to register multiple handles for a single signal.
class signal_manager_t {
public:
	err_t add(const std::vector<int>& signals, signal_event_t* handler);
	void remove(const std::vector<int>& signals, signal_event_t* handler);
	void ignore(int signum);

	// Single instance.
	static signal_manager_t* instance() {
		return &st_instance;
	}

private:
	// Make default constructor private.
	signal_manager_t() = default;

	// Remove copy constructors, operator=().
	signal_manager_t(const signal_manager_t&) = delete;
	signal_manager_t& operator=(const signal_manager_t&) = delete;

private:
	static signal_manager_t st_instance;
};


} // namespace c11httpd.

