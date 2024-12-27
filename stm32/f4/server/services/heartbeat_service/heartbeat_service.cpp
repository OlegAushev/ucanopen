#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/heartbeat_service/heartbeat_service.hpp>

namespace ucanopen {

HeartbeatService::HeartbeatService(impl::Server& server,
                                   std::chrono::milliseconds period)
        : _server(server), _period(period) {
    _id = calculate_cob_id(Cob::heartbeat, _server.node_id());
    _timepoint = emb::chrono::steady_clock::now();
}

void HeartbeatService::send() {
    if (_period.count() <= 0) {
        return;
    }

    auto now = emb::chrono::steady_clock::now();
    if (now >= _timepoint + _period) {
        can_payload payload = {};
        payload[0] = std::to_underlying(_server.nmt_state());
        _server._can_module.put_frame({_id, _len, payload});
        _timepoint = now;
    }
}

} // namespace ucanopen

#endif
#endif
