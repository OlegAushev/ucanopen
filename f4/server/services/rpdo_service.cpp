#ifdef STM32F4xx


#include "rpdo_service.h"


namespace ucanopen {

RpdoService::RpdoService(impl::Server& server)
        : _server(server) {
    for (auto i = 0; i < _rpdo_msgs.size(); ++i) {
        _rpdo_msgs[i].timeout = std::chrono::milliseconds(0);
        _rpdo_msgs[i].timepoint = mcu::chrono::system_clock::now();
        _rpdo_msgs[i].is_unhandled = false;
        _rpdo_msgs[i].handler = nullptr;
    }
}


void RpdoService::register_rpdo(CobRpdo rpdo, std::chrono::milliseconds timeout, void(*handler)(const can_payload&), can_id id) {
    if (id == 0) {
        id = calculate_cob_id(to_cob(rpdo), _server.node_id());
    }

    CAN_FilterTypeDef filter = {
        .FilterIdHigh = id << 5,
        .FilterIdLow = 0,
        .FilterMaskIdHigh = 0x7FF << 5,
        .FilterMaskIdLow = 0,
        .FilterFIFOAssignment = CAN_RX_FIFO0,
        .FilterBank = {},
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_32BIT,
        .FilterActivation = {},
        .SlaveStartFilterBank = {}
    };

    auto idx = std::to_underlying(rpdo);
    _rpdo_msgs[idx].attr = _server._can_module.register_message(filter);
    _rpdo_msgs[idx].timeout = timeout;
    _rpdo_msgs[idx].handler = handler;
}


std::vector<mcu::can::RxMessageAttribute> RpdoService::get_rx_attr() const {
    std::vector<mcu::can::RxMessageAttribute> attributes;
    for (const auto& rpdo : _rpdo_msgs) {
        if (rpdo.handler != nullptr) {
            attributes.push_back(rpdo.attr);
        }
    }
    return attributes;
}


FrameRecvStatus RpdoService::recv_frame(const mcu::can::RxMessageAttribute& attr, const can_frame& frame) {
    auto received_rpdo = std::find_if(_rpdo_msgs.begin(), _rpdo_msgs.end(),
                                      [attr](const auto& rpdo) { return rpdo.attr == attr; });
    if (received_rpdo == _rpdo_msgs.end()) {
        return FrameRecvStatus::attr_mismatch;
    }

    received_rpdo->timepoint = mcu::chrono::system_clock::now();
    received_rpdo->frame = frame;
    received_rpdo->is_unhandled = true;
    return FrameRecvStatus::success;
}


void RpdoService::handle_recv_frames() {
    for (auto& rpdo : _rpdo_msgs) {
        if (rpdo.is_unhandled && rpdo.handler != nullptr) {
            rpdo.handler(rpdo.frame.payload);
            rpdo.is_unhandled = false;
        }
    }
}

} // namespace ucanopen

#endif
