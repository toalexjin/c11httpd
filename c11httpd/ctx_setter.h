/**
 * Getter/Setter for context object.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/ctx.h"
#include <memory>


namespace c11httpd {


// Getter/Setter for context object.
class ctx_setter_t {
public:
	ctx_setter_t() = default;
	virtual ~ctx_setter_t() = default;

	virtual ctx_t* ctx() const;
	virtual void ctx(ctx_t* ctx);

private:
	ctx_setter_t(const ctx_setter_t&) = delete;
	ctx_setter_t& operator=(const ctx_setter_t&) = delete;

private:
	std::unique_ptr<ctx_t> m_ctx;
};


}

