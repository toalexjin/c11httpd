/**
 * Linux signal event handler.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <signal.h>


namespace c11httpd {


// Linux signal event.
class signal_event_t {
public:
	signal_event_t() {
	}

	virtual ~signal_event_t() = default;

	// Triggered when a Linux signal is received.
	virtual void on_signalled(int signum, siginfo_t* info, void* context);
};


} // namespace c11httpd.

