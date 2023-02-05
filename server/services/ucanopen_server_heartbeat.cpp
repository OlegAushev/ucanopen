#include <c28x_ucanopen/server/services/ucanopen_server_heartbeat.h>


namespace ucanopen {

ServerHeartbeatService::ServerHeartbeatService(impl::Server* server, uint64_t period_ms)
	: _server(server)
	, _period(period_ms)
{
	_timepoint = mcu::chrono::system_clock::now();
}

} // namespace ucanopen

