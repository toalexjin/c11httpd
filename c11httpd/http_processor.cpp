/**
 * HTTP processor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_processor.h"


namespace c11httpd {


uint32_t http_processor_t::on_connected(
	ctx_setter_t& ctx_setter, const conn_session_t& session,
	buf_t& send_buf) {
	return 0;
}

void http_processor_t::on_disconnected(
	ctx_setter_t& ctx_setter, const conn_session_t& session) {
}

uint32_t http_processor_t::on_received(
	ctx_setter_t& ctx_setter, const conn_session_t& session,
	buf_t& recv_buf, buf_t& send_buf) {

	// Create a new HTTP session object if it's not created.
	if (ctx_setter.ctx() == 0) {
		ctx_setter.ctx(new http_session_t());
	}

	// Get the HTTP session object.
	auto http_session = (http_session_t*) ctx_setter.ctx();

	// Parse HTTP request.
	size_t request_bytes;
	const auto parse_result = http_session->request().continue_to_parse(&recv_buf, &request_bytes);

	// HTTP request is not fully received, wait for next TCP packet.
	if (parse_result == http_request_t::parse_result_t::more) {
		return 0;
	}

	// HTTP request is incorrect, let's close the connection.
	if (parse_result == http_request_t::parse_result_t::failed) {
		return event_result_disconnect;
	}

	// Process this request.
	http_session->response().attach(&send_buf);
	const auto result = this->process_i(http_session);

	// We have processed this request, remove it from beginning of the buffer.
	// Because "request" has some fast_str_t point to the recv buffer,
	// we need to clear "request" first.
	http_session->request().clear();
	recv_buf.erase_front(request_bytes);

	if (result == rest_controller_t::result_t::disconnect) {
		return event_result_disconnect;
	}

	return 0;
}

rest_controller_t::result_t http_processor_t::process_i(http_session_t* http_session) {
	assert(http_session != 0);

	return rest_controller_t::result_t::done;
}

uint32_t http_processor_t::get_more_data(
	ctx_setter_t& ctx_setter, const conn_session_t& session, buf_t& send_buf) {
	return 0;
}


} // namespace c11httpd.
