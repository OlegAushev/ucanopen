#ifdef STM32F4xx


#include "node.h"
#include "../server/server.h"


namespace ucanopen {

Node::Node(Server& server)
        : _can_module(server._can_module) {
    server.add_node(this);
}


void Node::register_rx_message(CAN_FilterTypeDef& filter, std::chrono::milliseconds timeout, void(*handler)(const can_payload&)) {
    auto attr = _can_module.register_message(filter);
    _rx_messages.push_back({.attr = attr,
                            .timeout = timeout,
                            .timepoint = mcu::chrono::system_clock::now(),
                            .is_unhandled = false,
                            .frame= {},
                            .handler = handler});
}


void Node::register_tx_message(const CAN_TxHeaderTypeDef& header, std::chrono::milliseconds period, can_payload (*creator)()) {
    _tx_messages.push_back({.period = period,
                            .timepoint = mcu::chrono::system_clock::now(),
                            .header = header,
                            .creator = creator});
}


void Node::send() {
    auto now = mcu::chrono::system_clock::now();
    for (auto& message : _tx_messages) {
        if (message.period.count() <= 0) { continue; }
        if (now < message.timepoint + message.period) { continue; }

        can_payload payload = message.creator();
        _can_module.send(message.header, payload);
        message.timepoint = now;
    }
}


std::vector<mcu::can::MessageAttribute> Node::get_rx_attr() const {
    std::vector<mcu::can::MessageAttribute> attributes;
    for (const auto& msg : _rx_messages) {
        attributes.push_back(msg.attr);
    }
    return attributes;
}


FrameRecvStatus Node::recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) {
    auto received_msg = std::find_if(_rx_messages.begin(), _rx_messages.end(),
                                     [attr](const auto& msg){ return msg.attr == attr; });
    if (received_msg == _rx_messages.end()) {
        return FrameRecvStatus::attr_mismatch;
    }

    received_msg->timepoint = mcu::chrono::system_clock::now();
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
    auto now = mcu::chrono::system_clock::now();
    for (const auto& msg : _rx_messages) {
        if (now > msg.timepoint + msg.timeout) {
            return false;
        }
    }
    return true;
}

} // namespace ucanopen

#endif
