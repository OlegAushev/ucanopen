#ifdef STM32F4xx


#include "sync_service.h"


namespace ucanopen {

SyncService::SyncService(impl::Server& server, std::chrono::milliseconds period)
        : _server(server)
        , _period(period) {
    _timepoint = mcu::chrono::system_clock::now();
    _header = {
        .StdId = calculate_cob_id(CobType::sync, _server.node_id()),
        .ExtId = 0,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .DLC = 0,
        .TransmitGlobalTime = DISABLE
    };
}


void SyncService::send() {
    if (_period.count() <= 0) { return; }

    auto now = mcu::chrono::system_clock::now();
    if (now >= _timepoint + _period) {
        can_payload payload = {};
        _server._can_module.send(_header, payload);
        _timepoint = now;
    }
}
    
} // namespace ucanopen

#endif
