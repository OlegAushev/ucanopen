#ifdef MCUDRV_STM32
#ifdef STM32H7xx


#include "tpdo_service.h"


namespace ucanopen {


TpdoService::TpdoService(impl::Server& server)
        : _server(server) {
    for (size_t i = 0; i < _tpdo_msgs.size(); ++i) {
        _tpdo_msgs[i].period = std::chrono::milliseconds(0);
        _tpdo_msgs[i].timepoint = std::chrono::milliseconds(0);
        _tpdo_msgs[i].header = {
            .Identifier = calculate_cob_id(to_cob_type(CobTpdo(i)), _server.node_id()),
            .IdType = FDCAN_STANDARD_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8,
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };
        _tpdo_msgs[i].creator = nullptr;
    }
}


void TpdoService::register_tpdo(CobTpdo tpdo, std::chrono::milliseconds period, can_payload (*creator)()) {
    _tpdo_msgs[std::to_underlying(tpdo)].period = period;
    _tpdo_msgs[std::to_underlying(tpdo)].timepoint = emb::chrono::steady_clock::now();
    _tpdo_msgs[std::to_underlying(tpdo)].creator = creator;
}


void TpdoService::send() {
    auto now = emb::chrono::steady_clock::now();
    for (size_t i = 0; i < _tpdo_msgs.size(); ++i) {
        if (!_tpdo_msgs[i].creator || _tpdo_msgs[i].period.count() <= 0) { continue; }
        if (now < _tpdo_msgs[i].timepoint + _tpdo_msgs[i].period) { continue; }

        can_payload payload = _tpdo_msgs[i].creator();
        _server._can_module.put_frame(_tpdo_msgs[i].header, payload);
        _tpdo_msgs[i].timepoint = now;
    }
}


} // namespace ucanopen


#endif
#endif
