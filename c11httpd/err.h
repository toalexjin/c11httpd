/**
 * System error.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include <string>


namespace c11httpd {

namespace details__ {

struct err_impl_t {
	explicit err_impl_t(int err, bool shared = false)
	   : m_err(err), m_shared(shared) {
	}

	err_impl_t* clone() const {
		return new err_impl_t(this->m_err);
	}

	int m_err;

	// Indicates if this object is
	// pre-created and shared globally.
	bool m_shared;
};

} // namespace details__


/**
 * System error.
 */
class err_t {
public:
	err_t() : m_impl(0) {
	}

	err_t(int err) {
		this->m_impl = err == 0 ? 0 : create_i(err);
	}

	err_t(const err_t& another) {
		if (another.m_impl != 0 && !another.m_impl->m_shared) {
			this->m_impl = another.m_impl->clone();
		} else {
			this->m_impl = another.m_impl;
		}
	}

	err_t(err_t&& another) {
		this->m_impl = another.m_impl;
		another.m_impl = 0;
	}

	~err_t() {
		this->destroy();
	}

	void destroy() {
		if (this->m_impl == 0) {
			return;
		}

		if (!this->m_impl->m_shared) {
			delete this->m_impl;
		}

		this->m_impl = 0;
	}

	err_t& operator=(const err_t& another) {
		if (this == &another) {
			return *this;
		}

		this->set(another.get());
		return *this;
	}

	err_t& operator=(err_t&& another) {
		if (this == &another) {
			return *this;
		}

		this->m_impl = another.m_impl;
		another.m_impl = 0;
		return *this;
	}

	err_t& operator=(int err) {
		this->set(err);
		return *this;
	}

	bool ok() const {
		return this->m_impl == 0 ? true : false;
	}

	bool failed() const {
		return !this->ok();
	}

	bool operator!() const {
		return this->failed();
	}

	int get() const {
		return this->m_impl == 0 ? 0 : this->m_impl->m_err;
	}

	err_t& set(int err) {
		if (this->get() == err) {
			return *this;
		}

		if (err == 0) {
			this->destroy();
			return *this;
		}

		if (this->m_impl != 0 && !this->m_impl->m_shared) {
			this->m_impl->m_err = err;
			return *this;
		}

		this->destroy();
		this->m_impl = create_i(err);
		return *this;
	}

	err_t& set_ok() {
		return this->set(0);
	}

	err_t& set_current() {
		return this->set(err_t::current());
	}

	bool operator==(const err_t& another) const {
		return this->get() == another.get();
	}

	bool operator!=(const err_t& another) const {
		return this->get() != another.get();
	}

	bool operator==(int another) const {
		return this->get() == another;
	}

	bool operator!=(int another) const {
		return this->get() != another;
	}

	static int current();

private:
	static details__::err_impl_t* create_i(int err);

private:
	details__::err_impl_t* m_impl;
};


} // namespace c11httpd.


