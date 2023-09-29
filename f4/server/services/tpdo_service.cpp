#ifdef STM32F4xx


#include "tpdo_service.h"


namespace ucanopen {

TpdoService::TpdoService(impl::Server& server)
        : _server(server) {
    for (auto i = 0; i < _tpdo_msgs.size(); ++i) {
        _tpdo_msgs[i].period = std::chrono::milliseconds(0);
        _tpdo_msgs[i].timepoint = mcu::chrono::system_clock::now();
        _tpdo_msgs[i].header = {
            .StdId = calculate_cob_id(to_cob(CobTpdo(i)), _server.node_id()),
            .ExtId = 0,
            .IDE = CAN_ID_STD,
            .RTR = CAN_RTR_DATA,
            .DLC = 8,
            .TransmitGlobalTime = DISABLE
        };
        _tpdo_msgs[i].creator = nullptr;
    }
}


void TpdoService::register_tpdo(CobTpdo tpdo, std::chrono::milliseconds period, can_payload (*creator)()) {
    _tpdo_msgs[std::to_underlying(tpdo)].period = period;
    _tpdo_msgs[std::to_underlying(tpdo)].creator = creator;
}


void TpdoService::send() {
    auto now = mcu::chrono::system_clock::now();
    for (auto i = 0; i < _tpdo_msgs.size(); ++i) {
        if (!_tpdo_msgs[i].creator || _tpdo_msgs[i].period.count() <= 0) { continue; }
        if (now < _tpdo_msgs[i].timepoint + _tpdo_msgs[i].period) { continue; }

        can_payload payload = _tpdo_msgs[i].creator();
        _server._can_module.send(_tpdo_msgs[i].header, payload);
        _tpdo_msgs[i].timepoint = now;
    }
}

} // namespace ucanopen

#endif
