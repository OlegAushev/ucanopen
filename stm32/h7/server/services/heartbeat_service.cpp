#ifdef MCUDRV_STM32
#ifdef STM32H7xx

#include "heartbeat_service.h"

namespace ucanopen {

HeartbeatService::HeartbeatService(impl::Server& server,
                                   std::chrono::milliseconds period)
        : _server(server), _period(period) {
    _timepoint = emb::chrono::steady_clock::now();
    _header = {.Identifier =
                       calculate_cob_id(Cob::heartbeat, _server.node_id()),
               .IdType = FDCAN_STANDARD_ID,
               .TxFrameType = FDCAN_DATA_FRAME,
               .DataLength = FDCAN_DLC_BYTES_1,
               .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
               .BitRateSwitch = FDCAN_BRS_OFF,
               .FDFormat = FDCAN_CLASSIC_CAN,
               .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
               .MessageMarker = 0};
}

void HeartbeatService::send() {
    if (_period.count() <= 0) {
        return;
    }

    auto now = emb::chrono::steady_clock::now();
    if (now >= _timepoint + _period) {
        canpayload_t payload = {};
        payload[0] = std::to_underlying(_server.nmt_state());
        _server._can_module.put_frame(_header, payload);
        _timepoint = now;
    }
}

} // namespace ucanopen

#endif
#endif
