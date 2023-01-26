#pragma once


#include <mcu_c28x/f2837xd/chrono/mcu_chrono.h>
#include <ucanopen_c28x/server/impl/ucanopen_impl_server.h>



namespace ucanopen {

class ServerHeartbeatService
{
private:
	impl::Server* const _server;
	uint64_t _period;
	uint64_t _timepoint;
public:
	ServerHeartbeatService(impl::Server* server, uint64_t period_ms);

	void send()
	{
		assert(_server->_ipc_role == mcu::ipc::Role::primary);

		if (_period == 0) return;

		if (mcu::chrono::system_clock::now() >= _timepoint + _period)
		{
			can_payload payload;
			payload[0] = _server->nmt_state().underlying_value();
			_server->_can_module->send(CobType::heartbeat, payload.data, cob_sizes[CobType::heartbeat]);
			_timepoint = mcu::chrono::system_clock::now();
		}
	}
};

} // namespace ucanopen

