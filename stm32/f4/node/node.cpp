#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/node/node.hpp>
#include <ucanopen/stm32/f4/server/server.hpp>

namespace ucanopen {

Node::Node(Server& server) : can_module_(server.can_module_) {
    server.add_node(this);
}

#if defined(MCUDRV_STM32)
void Node::register_rx_message(CAN_FilterTypeDef& filter,
                               std::chrono::milliseconds timeout,
                               void (*handler)(const canpayload_t&)) {
#elif defined(MCUDRV_APM32)
void Node::register_rx_message(CAN_FilterConfig_T& filter,
                               std::chrono::milliseconds timeout,
                               void (*handler)(const canpayload_t&)) {
#endif
    auto attr = can_module_.register_rxmessage(filter);
    rx_messages_.push_back({.attr = attr,
                            .timeout = timeout,
                            .timepoint = emb::chrono::steady_clock::now(),
                            .unhandled = false,
                            .frame = {},
                            .handler = handler});
}

void Node::register_tx_message(canid_t id,
                               uint8_t len,
                               std::chrono::milliseconds period,
                               canpayload_t (*creator)()) {
    tx_messages_.push_back({.period = period,
                            .timepoint = emb::chrono::steady_clock::now(),
                            .id = id,
                            .len = len,
                            .creator = creator});
}

void Node::send() {
    auto now = emb::chrono::steady_clock::now();
    for (auto& message : tx_messages_) {
        if (message.period.count() <= 0) {
            continue;
        }
        if (now < message.timepoint + message.period) {
            continue;
        }

        canpayload_t payload = message.creator();
        can_module_.put_frame({message.id, message.len, payload});
        message.timepoint = now;
    }
}

std::vector<ucan::RxMessageAttribute> Node::get_rx_attr() const {
    std::vector<ucan::RxMessageAttribute> attributes;
    for (const auto& msg : rx_messages_) {
        attributes.push_back(msg.attr);
    }
    return attributes;
}

void Node::recv(const ucan::RxMessageAttribute& attr, const can_frame& frame) {
    auto received_msg =
            std::find_if(rx_messages_.begin(),
                         rx_messages_.end(),
                         [attr](const auto& msg) { return msg.attr == attr; });
    if (received_msg == rx_messages_.end()) {
        return;
    }

    if (received_msg->unhandled) {
        return;
    }

    received_msg->timepoint = emb::chrono::steady_clock::now();
    received_msg->frame = frame;
    received_msg->unhandled = true;
}

void Node::handle() {
    for (auto& msg : rx_messages_) {
        if (msg.unhandled && msg.handler != nullptr) {
            msg.handler(msg.frame.payload);
            msg.unhandled = false;
        }
    }
}

bool Node::good() {
    auto now = emb::chrono::steady_clock::now();
    for (const auto& msg : rx_messages_) {
        if (now > msg.timepoint + msg.timeout) {
            return false;
        }
    }
    return true;
}

} // namespace ucanopen

#endif
#endif
