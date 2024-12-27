#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/node/node.hpp>
#include <ucanopen/stm32/f4/server/server.hpp>

namespace ucanopen {

Node::Node(Server& server) : _can_module(server._can_module) {
    server.add_node(this);
}

#if defined(MCUDRV_STM32)
void Node::register_rx_message(CAN_FilterTypeDef& filter,
                               std::chrono::milliseconds timeout,
                               void (*handler)(const can_payload&)) {
#elif defined(MCUDRV_APM32)
void Node::register_rx_message(CAN_FilterConfig_T& filter,
                               std::chrono::milliseconds timeout,
                               void (*handler)(const can_payload&)) {
#endif
    auto attr = _can_module.register_rxmessage(filter);
    _rx_messages.push_back({.attr = attr,
                            .timeout = timeout,
                            .timepoint = emb::chrono::steady_clock::now(),
                            .is_unhandled = false,
                            .frame = {},
                            .handler = handler});
}

void Node::register_tx_message(can_id id,
                               uint8_t len,
                               std::chrono::milliseconds period,
                               can_payload (*creator)()) {
    _tx_messages.push_back({.period = period,
                            .timepoint = emb::chrono::steady_clock::now(),
                            .id = id,
                            .len = len,
                            .creator = creator});
}

void Node::send() {
    auto now = emb::chrono::steady_clock::now();
    for (auto& message : _tx_messages) {
        if (message.period.count() <= 0) {
            continue;
        }
        if (now < message.timepoint + message.period) {
            continue;
        }

        can_payload payload = message.creator();
        _can_module.put_frame({message.id, message.len, payload});
        message.timepoint = now;
    }
}

std::vector<ucan::RxMessageAttribute> Node::get_rx_attr() const {
    std::vector<ucan::RxMessageAttribute> attributes;
    for (const auto& msg : _rx_messages) {
        attributes.push_back(msg.attr);
    }
    return attributes;
}

FrameRecvStatus Node::recv_frame(const ucan::RxMessageAttribute& attr,
                                 const can_frame& frame) {
    auto received_msg =
            std::find_if(_rx_messages.begin(),
                         _rx_messages.end(),
                         [attr](const auto& msg) { return msg.attr == attr; });
    if (received_msg == _rx_messages.end()) {
        return FrameRecvStatus::attr_mismatch;
    }

    received_msg->timepoint = emb::chrono::steady_clock::now();
    received_msg->frame = frame;
    received_msg->is_unhandled = true;
    return FrameRecvStatus::success;
}

void Node::handle_recv_frames() {
    for (auto& msg : _rx_messages) {
        if (msg.is_unhandled && msg.handler != nullptr) {
            msg.handler(msg.frame.payload);
            msg.is_unhandled = false;
        }
    }
}

bool Node::connection_ok() {
    auto now = emb::chrono::steady_clock::now();
    for (const auto& msg : _rx_messages) {
        if (now > msg.timepoint + msg.timeout) {
            return false;
        }
    }
    return true;
}

} // namespace ucanopen

#endif
#endif
