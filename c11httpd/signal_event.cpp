/**
 * Linux signal event handler.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/signal_event.h"


namespace c11httpd {


void signal_event_t::on_signal(int signum) {
	(void) &signum;
};


} // namespace c11httpd.

