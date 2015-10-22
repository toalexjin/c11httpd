/**
 * HTTP processor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_processor.h"


namespace c11httpd {


uint32_t http_processor_t::on_connected(conn_session_t* session, buf_t* send_buf) {
	return 0;
}

void http_processor_t::on_disconnected(conn_session_t* session) {
}

uint32_t http_processor_t::on_received(conn_session_t* session,
			buf_t* recv_buf, buf_t* send_buf) {

	// Create a new HTTP session object if it's not created.
	if (session->get_ctx() == 0) {
		session->set_ctx(new http_session_t());
	}

	// Get the HTTP session object.
	auto http_session = (http_session_t*) session->get_ctx();

	// Parse HTTP request.
	size_t request_bytes;
	const auto parse_result = http_session->request().continue_to_parse(recv_buf, &request_bytes);

	// HTTP request is not fully received, wait for next TCP packet.
	if (parse_result == http_request_t::parse_result_t::more) {
		return 0;
	}

	// HTTP request is incorrect, let's close the connection.
	if (parse_result == http_request_t::parse_result_t::failed) {
		return event_result_disconnect;
	}

	// Handle this request.
	const auto result = this->process_i(http_session);

	// We have processed this request, remove it from beginning of the buffer.
	recv_buf->erase_front(request_bytes);

	if (result == rest_controller_t::result_t::disconnect) {
		return event_result_disconnect;
	}

	return 0;
}

rest_controller_t::result_t http_processor_t::process_i(http_session_t* http_session) {
	return rest_controller_t::result_t::done;
}

uint32_t http_processor_t::get_more_data(conn_session_t* session, buf_t* send_buf) {
	return 0;
}

} // namespace c11httpd.
