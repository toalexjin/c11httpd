/**
 * HTTP processor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/config.h"
#include "c11httpd/conn_event.h"
#include "c11httpd/http_conn.h"
#include "c11httpd/rest_ctrl.h"
#include <vector>


namespace c11httpd {


// HTTP Processor.
//
// This class does following:
// -# Parse HTTP request header and content.
// -# Distribute request to the right controller based on URI.
class http_processor_t : public conn_event_t {
public:
	explicit http_processor_t(const std::vector<rest_ctrl_t*>& controllers)
		: m_controllers(controllers) {
	}
	virtual ~http_processor_t() = default;

	virtual uint32_t on_connected(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		conn_session_t& session,
		buf_t& send_buf);

	virtual void on_disconnected(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		conn_session_t& session);

	virtual uint32_t on_received(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		conn_session_t& session,
		buf_t& recv_buf, buf_t& send_buf);

	virtual uint32_t get_more_data(
		ctx_setter_t& ctx_setter, const config_t& cfg,
		conn_session_t& session,
		buf_t& send_buf);

private:
	// Process a HTTP request.
	//
	// @return A value of rest_result_t.
	rest_result_t process_i(
		const config_t& cfg, conn_session_t& session,
		http_conn_t* http_conn, buf_t* send_buf);

private:
	const std::vector<rest_ctrl_t*> m_controllers;
};


}

