#include <c28x_ucanopen/server/services/ucanopen_server_heartbeat.h>


namespace ucanopen {

ServerHeartbeatService::ServerHeartbeatService(impl::Server* server, emb::chrono::milliseconds period)
	: _server(server)
	, _period(period)
{
	_timepoint = mcu::chrono::system_clock::now();
}

} // namespace ucanopen

