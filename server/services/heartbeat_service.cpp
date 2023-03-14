#include "heartbeat_service.h"


namespace ucanopen {

HeartbeatService::HeartbeatService(impl::Server* server, emb::chrono::milliseconds period)
		: _server(server)
		, _period(period) {
	_timepoint = mcu::chrono::system_clock::now();
}

} // namespace ucanopen

