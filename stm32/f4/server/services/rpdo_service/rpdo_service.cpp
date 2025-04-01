#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/rpdo_service/rpdo_service.hpp>

namespace ucanopen {

RpdoService::RpdoService(impl::Server& server) : server_(server) {
    for (size_t i = 0; i < messages_.size(); ++i) {
        messages_[i].timeout = std::chrono::milliseconds(0);
        messages_[i].timepoint = std::chrono::milliseconds(0);
        messages_[i].unhandled = false;
        messages_[i].handler = nullptr;
    }
}

void RpdoService::register_rpdo(CobRpdo rpdo,
                                std::chrono::milliseconds timeout,
                                void (*handler)(const canpayload_t&),
                                canid_t id) {
    if (id == 0) {
        id = calculate_cob_id(to_cob(rpdo), server_.node_id());
    }
#if defined(MCUDRV_STM32)
    CAN_FilterTypeDef filter = {.FilterIdHigh = id << 5,
                                .FilterIdLow = 0,
                                .FilterMaskIdHigh = 0x7FF << 5,
                                .FilterMaskIdLow = 0,
                                .FilterFIFOAssignment = CAN_RX_FIFO0,
                                .FilterBank = {},
                                .FilterMode = CAN_FILTERMODE_IDMASK,
                                .FilterScale = CAN_FILTERSCALE_32BIT,
                                .FilterActivation = {},
                                .SlaveStartFilterBank = {}};
#elif defined(MCUDRV_APM32)
    CAN_FilterConfig_T filter = {.filterNumber{},
                                 .filterIdHigh = uint16_t(uint16_t(id) << 5),
                                 .filterIdLow = 0,
                                 .filterMaskIdHigh = 0x7FF << 5,
                                 .filterMaskIdLow = 0,
                                 .filterActivation{},
                                 .filterFIFO = CAN_FILTER_FIFO_0,
                                 .filterMode = CAN_FILTER_MODE_IDMASK,
                                 .filterScale = CAN_FILTER_SCALE_32BIT};
#endif

    auto idx = std::to_underlying(rpdo);
    messages_[idx].attr = server_.can_module_.register_rxmessage(filter);
    messages_[idx].timeout = timeout;
    messages_[idx].timepoint = emb::chrono::steady_clock::now();
    messages_[idx].handler = handler;
}

std::vector<ucan::RxMessageAttribute> RpdoService::get_rx_attr() const {
    std::vector<ucan::RxMessageAttribute> attributes;
    for (const auto& msg : messages_) {
        if (msg.handler != nullptr) {
            attributes.push_back(msg.attr);
        }
    }
    return attributes;
}

void RpdoService::recv(const ucan::RxMessageAttribute& attr,
                       const can_frame& frame) {
    auto msg_it = std::find_if(
            messages_.begin(), messages_.end(), [attr](const auto& rpdo) {
                return rpdo.attr == attr;
            });
    if (msg_it == messages_.end()) {
        return;
    }

    if (msg_it->unhandled) {
        server_.on_rpdo_overrun();
        return;
    }

    msg_it->timepoint = emb::chrono::steady_clock::now();
    msg_it->frame = frame;
    msg_it->unhandled = true;
}

void RpdoService::handle() {
    for (auto& msg : messages_) {
        if (msg.unhandled && msg.handler != nullptr) {
            msg.handler(msg.frame.payload);
            msg.unhandled = false;
        }
    }
}

} // namespace ucanopen

#endif
#endif
