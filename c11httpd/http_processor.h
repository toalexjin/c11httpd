/**
 * HTTP processor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"
#include "c11httpd/conn_event.h"
#include "c11httpd/http_session.h"
#include "c11httpd/rest_controller.h"
#include <vector>


namespace c11httpd {


// HTTP Processor.
//
// This class does following:
// -# Parse HTTP request header and content.
// -# Distribute request to the right controller based on URI.
class http_processor_t : public conn_event_t {
public:
	explicit http_processor_t(const std::vector<rest_controller_t*>& controllers)
		: m_controllers(controllers) {
	}
	virtual ~http_processor_t() = default;

	virtual uint32_t on_connected(conn_session_t& session, buf_t& send_buf);
	virtual void on_disconnected(conn_session_t& session);
	virtual uint32_t on_received(conn_session_t& session,
			buf_t& recv_buf, buf_t& send_buf);
	virtual uint32_t get_more_data(conn_session_t& session, buf_t& send_buf);

private:
	// Process a HTTP request.
	//
	// @return A value of rest_controller_t::result_t.
	rest_controller_t::result_t process_i(http_session_t* http_session);

private:
	const std::vector<rest_controller_t*>& m_controllers;
};


}

