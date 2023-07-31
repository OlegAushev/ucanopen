#ifdef STM32F4xx


#include "tpdo_service.h"


namespace ucanopen {

TpdoService::TpdoService(impl::Server& server)
        : _server(server) {
    for (auto i = 0; i < _tpdo_list.size(); ++i) {
        _tpdo_list[i].period = std::chrono::milliseconds(0);
        _tpdo_list[i].timepoint = mcu::chrono::system_clock::now();
        _tpdo_list[i].header = {
            .Identifier = calculate_cob_id(to_cob_type(TpdoType(i)), _server.node_id()),
            .IdType = FDCAN_STANDARD_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8,
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };
        _tpdo_list[i].creator = nullptr;
    }
}


void TpdoService::register_tpdo(TpdoType tpdo_type, std::chrono::milliseconds period, can_payload (*creator)()) {
    _tpdo_list[std::to_underlying(tpdo_type)].period = period;
    _tpdo_list[std::to_underlying(tpdo_type)].creator = creator;
}


void TpdoService::send() {
    auto now = mcu::chrono::system_clock::now();
    for (auto i = 0; i < _tpdo_list.size(); ++i) {
        if (!_tpdo_list[i].creator || _tpdo_list[i].period.count() <= 0) { continue; }
        if (now < _tpdo_list[i].timepoint + _tpdo_list[i].period) { continue; }

        can_payload payload = _tpdo_list[i].creator();
        _server._can_module.send(_tpdo_list[i].header, payload);
        _tpdo_list[i].timepoint = now;
    }
}

} // namespace ucanopen

#endif
