#ifdef MCUDRV_C28X

#include <ucanopen/c28x/server/services/heartbeat_service.hpp>

namespace ucanopen {

HeartbeatService::HeartbeatService(impl::Server& server, emb::chrono::milliseconds period)
        : server_(server)
        , period_(period) {
    timepoint_ = emb::chrono::steady_clock::now();
}

void HeartbeatService::send() {
    if (period_.count() <= 0)  { return; }

    emb::chrono::milliseconds now = emb::chrono::steady_clock::now();
    if (now >= timepoint_ + period_) {
        can_payload payload;
        payload[0] = server_.nmt_state().underlying_value();
        server_.can_module_.send(Cob::heartbeat,
                                 payload.data,
                                 cob_data_len[Cob::heartbeat]);
        timepoint_ = now;
    }
}

} // namespace ucanopen

#endif
