#ifdef STM32F4xx


#include "sync_service.h"


namespace ucanopen {

SyncService::SyncService(impl::Server& server, std::chrono::milliseconds period)
        : _server(server)
        , _period(period) {
    _timepoint = mcu::chrono::system_clock::now();
    _header = {
        .Identifier = calculate_cob_id(CobType::sync, _server.node_id()),
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = FDCAN_DLC_BYTES_0,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker = 0
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
