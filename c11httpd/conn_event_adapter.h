/**
 * Client connection event adapter.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/config.h"
#include "c11httpd/conn_event.h"
#include <functional>


namespace c11httpd {


// Client connection event adapter.
//
// With this class, you could quickly create an event handler by using c++ lambda.
class conn_event_adapter_t : public conn_event_t {
public:
	typedef std::function<uint32_t(ctx_setter_t&, const config_t&, const conn_session_t&, buf_t&)> on_connected_t;
	typedef std::function<void(ctx_setter_t&, const config_t&, const conn_session_t&)> on_disconnected_t;
	typedef std::function<uint32_t(ctx_setter_t&, const config_t&, const conn_session_t&, buf_t&, buf_t&)> on_received_t;
	typedef std::function<uint32_t(ctx_setter_t&, const config_t&, const conn_session_t&, buf_t&)> get_more_data_t;

public:
	conn_event_adapter_t() = default;
	virtual ~conn_event_adapter_t() = default;

	// Getter/Setter for lambda functions.
	const on_connected_t& lambda_on_connected() const {
		return this->m_on_connected;
	}

	void lambda_on_connected(const on_connected_t& handler) {
		this->m_on_connected = handler;
	}

	const on_disconnected_t& lambda_on_disconnected() const {
		return this->m_on_disconnected;
	}

	void lambda_on_disconnected(const on_disconnected_t& handler) {
		this->m_on_disconnected = handler;
	}

	const on_received_t& lambda_on_received() const {
		return this->m_on_received;
	}

	void lambda_on_received(const on_received_t& handler) {
		this->m_on_received = handler;
	}

	const get_more_data_t& lambda_get_more_data() const {
		return this->m_get_more_data;
	}

	void lambda_get_more_data(const get_more_data_t& handler) {
		this->m_get_more_data = handler;
	}

	// Event callback functions.
	virtual uint32_t on_connected(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		const conn_session_t& session, buf_t& send_buf);

	virtual void on_disconnected(ctx_setter_t& ctx_setter,
		const config_t& cfg, const conn_session_t& session);

	virtual uint32_t on_received(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		const conn_session_t& session,
		buf_t& recv_buf, buf_t& send_buf);

	virtual uint32_t get_more_data(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		const conn_session_t& session, buf_t& send_buf);

private:
	on_connected_t m_on_connected;
	on_disconnected_t m_on_disconnected;
	on_received_t m_on_received;
	get_more_data_t m_get_more_data;
};


} // namespace c11httpd.

