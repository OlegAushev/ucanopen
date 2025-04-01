#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/heartbeat_service/heartbeat_service.hpp>

namespace ucanopen {

HeartbeatService::HeartbeatService(impl::Server& server,
                                   std::chrono::milliseconds period)
        : server_(server), period_(period) {
    id_ = calculate_cob_id(Cob::heartbeat, server_.node_id());
    timepoint_ = emb::chrono::steady_clock::now();
}

void HeartbeatService::send() {
    if (period_.count() <= 0) {
        return;
    }

    auto now = emb::chrono::steady_clock::now();
    if (now >= timepoint_ + period_) {
        canpayload_t payload = {};
        payload[0] = std::to_underlying(server_.nmt_state());
        server_.can_module_.put_frame({id_, len_, payload});
        timepoint_ = now;
    }
}

} // namespace ucanopen

#endif
#endif
