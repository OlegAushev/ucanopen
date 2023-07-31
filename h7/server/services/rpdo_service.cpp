#ifdef STM32H7xx


#include "rpdo_service.h"


namespace ucanopen {

RpdoService::RpdoService(impl::Server& server)
        : _server(server) {
    for (auto i = 0; i < _rpdo_list.size(); ++i) {
        _rpdo_list[i].timeout = std::chrono::milliseconds(0);
        _rpdo_list[i].timepoint = mcu::chrono::system_clock::now();
        _rpdo_list[i].is_unhandled = false;
        _rpdo_list[i].handler = nullptr;
    }
}


void RpdoService::register_rpdo(RpdoType rpdo_type, std::chrono::milliseconds timeout, void(*handler)(const can_payload&), can_id id) {
    if (id == 0) {
        id = calculate_cob_id(to_cob_type(rpdo_type), _server.node_id());
    }

    FDCAN_FilterTypeDef filter = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_MASK,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = id,
        .FilterID2 = 0x7FF,
        .RxBufferIndex = 0,
        .IsCalibrationMsg = 0
    };

    auto idx = std::to_underlying(rpdo_type);
    _rpdo_list[idx].attr = _server._can_module.register_message(filter);
    _rpdo_list[idx].timeout = timeout;
    _rpdo_list[idx].handler = handler;
}


std::vector<mcu::can::MessageAttribute> RpdoService::get_rx_attr() const {
    std::vector<mcu::can::MessageAttribute> attributes;
    for (const auto& rpdo : _rpdo_list) {
        if (rpdo.handler != nullptr) {
            attributes.push_back(rpdo.attr);
        }
    }
    return attributes;
}


FrameRecvStatus RpdoService::recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) {
    auto received_rpdo = std::find_if(_rpdo_list.begin(), _rpdo_list.end(),
                                      [attr](const auto& rpdo) { return rpdo.attr == attr; });
    if (received_rpdo == _rpdo_list.end()) {
        return FrameRecvStatus::attr_mismatch;
    }

    received_rpdo->timepoint = mcu::chrono::system_clock::now();
    received_rpdo->frame = frame;
    received_rpdo->is_unhandled = true;
    return FrameRecvStatus::success;
}


void RpdoService::handle_recv_frames() {
    for (auto& rpdo : _rpdo_list) {
        if (rpdo.is_unhandled && rpdo.handler != nullptr) {
            rpdo.handler(rpdo.frame.payload);
            rpdo.is_unhandled = false;
        }
    }
}


bool RpdoService::connection_ok() {
    auto now = mcu::chrono::system_clock::now();
    for (const auto& rpdo : _rpdo_list) {
        if (now > rpdo.timepoint + rpdo.timeout) {
            return false;
        }
    }
    return true;
}

} // namespace ucanopen

#endif
