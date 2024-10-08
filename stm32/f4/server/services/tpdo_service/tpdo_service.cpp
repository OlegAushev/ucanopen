#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include "tpdo_service.h"


namespace ucanopen {


TpdoService::TpdoService(impl::Server& server)
        : _server(server) {
    for (size_t i = 0; i < _tpdo_msgs.size(); ++i) {
        _tpdo_msgs[i].id = calculate_cob_id(to_cob(CobTpdo(i)), _server.node_id());
        _tpdo_msgs[i].period = std::chrono::milliseconds(0);
        _tpdo_msgs[i].timepoint = std::chrono::milliseconds(0);
        _tpdo_msgs[i].creator = nullptr;
    }
}


void TpdoService::register_tpdo(CobTpdo tpdo, std::chrono::milliseconds period, can_payload (*creator)()) {
    _tpdo_msgs[std::to_underlying(tpdo)].period = period;
    _tpdo_msgs[std::to_underlying(tpdo)].timepoint = mcu::chrono::steady_clock::now();
    _tpdo_msgs[std::to_underlying(tpdo)].creator = creator;
}


void TpdoService::send() {
    auto now = mcu::chrono::steady_clock::now();
    for (auto& msg : _tpdo_msgs) {
        if (!msg.creator || msg.period.count() <= 0) { continue; }
        if (now < msg.timepoint + msg.period) { continue; }

        can_payload payload = msg.creator();
        _server._can_module.put_frame({msg.id, msg.len, payload});
        msg.timepoint = now;
    }
}


} // namespace ucanopen


#endif
#endif
