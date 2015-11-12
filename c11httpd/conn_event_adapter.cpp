/**
 * Client connection event adapter.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/conn_event_adapter.h"


namespace c11httpd {


uint32_t conn_event_adapter_t::on_connected(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session, buf_t& send_buf) {
	if (this->m_on_connected) {
		return this->m_on_connected(ctx_setter, cfg, session, send_buf);
	} else {
		return 0;
	}
}

void conn_event_adapter_t::on_disconnected(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session) {
	if (this->m_on_disconnected) {
		this->m_on_disconnected(ctx_setter, cfg, session);
	}
}

uint32_t conn_event_adapter_t::on_received(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session,
	buf_t& recv_buf, buf_t& send_buf) {
	if (this->m_on_received) {
		return this->m_on_received(ctx_setter, cfg, session, recv_buf, send_buf);
	} else {
		recv_buf.clear();
		return 0;
	}
}

uint32_t conn_event_adapter_t::get_more_data(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session,
	buf_t& send_buf) {
	if (this->m_get_more_data) {
		return this->m_get_more_data(ctx_setter, cfg, session, send_buf);
	} else {
		return 0;
	}
}

uint32_t conn_event_adapter_t::on_aio_completed(
	ctx_setter_t& ctx_setter, const config_t& cfg,
	conn_session_t& session,
	int running_count,
	const std::vector<aio_t>& completed,
	buf_t& send_buf) {

	if (this->m_on_aio_completed) {
		return this->m_on_aio_completed(ctx_setter, cfg,
			session, running_count, completed, send_buf);
	} else {
		return 0;
	}
}


} // namespace c11httpd.

