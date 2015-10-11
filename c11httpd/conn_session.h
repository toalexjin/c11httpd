/**
 * Client session.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/buf.h"
#include "c11httpd/conn_ctx.h"
#include <memory>
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

	// Get user context.
	//
	// When "on_connected" event happens, conn_session_t::get_ctx()
	// might return non-null. The returned object should be re-used
	// by user in order to avoid create context object every time.
	//
	// When "on_disconnected" event happens, user should NOT free
	// the context object because the library need to save it for
	// later use. After "on_disconnected" event completed, the library
	// would call conn_ctx_t::clear() so that the context object
	// could be re-used next time.
	//
	// @see conn_ctx_t::clear().
	conn_ctx_t* get_ctx() const {
		return this->m_ctx.get();
	}

	// Set user context.
	//
	// When "on_connected" event happens and conn_session_t::get_ctx()
	// returns null, then user should create a new conn_ctx_t object
	// associated with conn_session_t by calling this function.
	void set_ctx(conn_ctx_t* ctx) {
		this->m_ctx = std::unique_ptr<conn_ctx_t>(ctx);
	}

private:
	std::unique_ptr<conn_ctx_t> m_ctx;
};


template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(
		std::basic_ostream<Char, Traits>& ostream, const conn_session_t& s) {
	ostream << s.ip() << ":" << s.port() << "(" << (const void*) &s << ")";

	return ostream;
}


}

