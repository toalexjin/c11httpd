/**
 * Getter/Setter for context object.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/ctx_setter.h"


namespace c11httpd {


ctx_t* ctx_setter_t::ctx() const {
	return this->m_ctx.get();
}

void ctx_setter_t::ctx(ctx_t* ctx) {
	this->m_ctx = std::unique_ptr<ctx_t>(ctx);
}


} // namespace c11httpd.

