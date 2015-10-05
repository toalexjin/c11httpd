/**
 * System error.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/err.h"
#include <cerrno>


namespace c11httpd {


int err_t::current() {
	return errno;
}

details__::err_impl_t* err_t::create_i(int err) {
	assert(err != 0);

	// TO-DO:
	//
	// Search "err" in pre-created error objects.

	return new details__::err_impl_t(err);
}

} // namespace c11httpd.

