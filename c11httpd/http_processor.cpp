/**
 * HTTP processor.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/http_processor.h"


namespace c11httpd {


uint32_t http_processor_t::on_connected(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session,
	buf_t& send_buf) {
	return 0;
}

void http_processor_t::on_disconnected(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session) {
}

uint32_t http_processor_t::on_received(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session,
	buf_t& recv_buf, buf_t& send_buf) {

	// Create a new HTTP session object if it's not created.
	if (ctx_setter.ctx() == 0) {
		ctx_setter.ctx(new http_conn_t());
	}

	// Get the HTTP session object.
	auto http_conn = (http_conn_t*) ctx_setter.ctx();

	// Parse HTTP request.
	size_t request_bytes;
	const auto parse_result = http_conn->request().continue_to_parse(&recv_buf, &request_bytes);

	// HTTP request is not fully received, wait for next TCP packet.
	if (parse_result == http_request_t::parse_result_t::more) {
		return 0;
	}

	// HTTP request is incorrect, let's close the connection.
	if (parse_result == http_request_t::parse_result_t::failed) {
		return conn_event_t::result_disconnect;
	}

	// Save the original size of "send_buf".
	const auto old_size = send_buf.size();

	// Process this request.
	const auto result = this->process_i(cfg, session, http_conn, &send_buf);

	// If fatal error happens, then restore original size of "send_buf".
	if (result == rest_result_t::abandon) {
		send_buf.size(old_size);
		return conn_event_t::result_disconnect;
	}

	// We have processed this request, remove it from beginning of the buffer.
	// Because "request" has some fast_str_t point to the recv buffer,
	// we need to clear "request" first.
	http_conn->request().clear();
	recv_buf.erase_front(request_bytes);

	return 0;
}

rest_result_t http_processor_t::process_i(
	const config_t& cfg, conn_session_t& session,
	http_conn_t* http_conn, buf_t* send_buf) {
	assert(http_conn != 0);

	rest_ctrl_t* controller = *(this->m_controllers.begin());
	const rest_ctrl_t::api_t& api = *(controller->apis().begin());

	// Attach response object to send_buf.
	http_conn->response().attach(&cfg,
		&(http_conn->request()), &(std::get<4>(api)), send_buf);

	const auto result = std::get<2>(api)->invoke(*http_conn, session,
		http_conn->request(), std::vector<fast_str_t>(),
		http_conn->response());

	// Detach response object from send_buf.
	http_conn->response().detach(result);

	return result;
}

uint32_t http_processor_t::get_more_data(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session, buf_t& send_buf) {
	return 0;
}

uint32_t http_processor_t::on_aio_completed(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session,
	const std::vector<aio_t>& completed,
	buf_t& send_buf) {

	return 0;
}



} // namespace c11httpd.
