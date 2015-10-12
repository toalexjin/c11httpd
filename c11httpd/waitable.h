/**
 * Waitable interface.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"


namespace c11httpd {

// Waitable interface.
//
// epoll could monitor listening-socket, connection-socket and signal.
// This is the base class/interface of these three types of objects.
class waitable_t {
public:
	enum type_t {
		type_listen,
		type_conn,
		type_signal
	};

public:
	explicit waitable_t(type_t type) : m_type(type) {
	}

	virtual ~waitable_t() = default;

	// Return a value of type_t.
	type_t wait_type() const {
		return this->m_type;
	}

	// Get file descriptor.
	virtual int fd() const = 0;

private:
	const type_t m_type;
};


} // namespace c11httpd.
