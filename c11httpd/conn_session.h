/**
 * Client session.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <string>


namespace c11httpd {


// Client session.
//
// This is client session interface, implemented by conn_t.
class conn_session_t {
public:
	conn_session_t() = default;
	virtual ~conn_session_t() = default;

	virtual const std::string& ip() const = 0;
	virtual uint16_t port() const = 0;
	virtual bool ipv6() const = 0;
};


template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(
		std::basic_ostream<Char, Traits>& ostream, const conn_session_t& s) {
	ostream << s.ip() << ":" << s.port() << "(" << (const void*) &s << ")";

	return ostream;
}


}

