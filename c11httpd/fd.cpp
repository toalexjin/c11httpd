/**
 * Socket.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/fd.h"
#include <unistd.h>


namespace c11httpd {


err_t fd_t::close() {
	err_t ret;

	if (!this->is_closed()) {
		if (::close(this->m_handle) == 0) {
			this->m_handle = -1;
		} else {
			ret.set_current();
		}
	}

	return ret;
}


} // namespace c11httpd.


