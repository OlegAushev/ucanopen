#ifdef MCUDRV_C28X

#include <ucanopen/c28x/server/services/tpdo_service.hpp>

namespace ucanopen {

TpdoService::TpdoService(impl::Server& server)
        : server_(server) {
    for (size_t i = 0; i < tpdo_msgs_.size(); ++i) {
        tpdo_msgs_[i].period = emb::chrono::milliseconds(0);
        tpdo_msgs_[i].timepoint = emb::chrono::milliseconds(0);
        tpdo_msgs_[i].creator = NULL;
    }
}

void TpdoService::register_tpdo(CobTpdo tpdo,
                                emb::chrono::milliseconds period,
                                canpayload_t (*creator)()) {
    const size_t idx = tpdo.underlying_value();
    tpdo_msgs_[idx].period = period;
    tpdo_msgs_[idx].timepoint = emb::chrono::steady_clock::now();
    tpdo_msgs_[idx].creator = creator;
}

void TpdoService::send() {
    emb::chrono::milliseconds now = emb::chrono::steady_clock::now();

    for (size_t i = 0; i < tpdo_msgs_.size(); ++i) {
        if (!tpdo_msgs_[i].creator || tpdo_msgs_[i].period.count() <= 0) {
            continue;
        }

        if (now < tpdo_msgs_[i].timepoint + tpdo_msgs_[i].period) {
            continue;
        }

        const canpayload_t payload = tpdo_msgs_[i].creator();
        const Cob cob = to_cob(CobTpdo(i));
        server_.can_module_.send(cob.underlying_value(),
                                 payload.data,
                                 cob_data_len[cob.underlying_value()]);
        tpdo_msgs_[i].timepoint = now;
    }
}

} // namespace ucanopen

#endif
