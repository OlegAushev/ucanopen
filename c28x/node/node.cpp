#ifdef MCUDRV_C28X

#include <ucanopen/c28x/node/node.hpp>
#include <ucanopen/c28x/server/server.hpp>

namespace ucanopen {

Node::Node(Server& server) : server_(server) {
    server_.add_node(this);
}

void Node::register_rx_message(canid_t id,
                               uint16_t len,
                               emb::chrono::milliseconds timeout,
                               void (*handler)(const canpayload_t&)) {
    assert(!server_.message_objects_.full());

    mcu::c28x::can::MessageObject msg_obj;
    msg_obj.obj_id = server_.message_objects_.size();
    msg_obj.frame_id = id;
    if (id <= 0x7FF) {
        msg_obj.frame_type = CAN_MSG_FRAME_STD;
    } else {
        msg_obj.frame_type = CAN_MSG_FRAME_EXT;
    }
    msg_obj.obj_type = CAN_MSG_OBJ_TYPE_RX;
    msg_obj.frame_idmask = 0;
    msg_obj.flags = CAN_MSG_OBJ_RX_INT_ENABLE;
    msg_obj.data_len = len;
    server_.message_objects_.push_back(msg_obj);
    server_.can_module_.setup_message_object(msg_obj);

    server_.register_node_cob(this, msg_obj.obj_id - cob_count);

    RxMessage rx_msg;
    rx_msg.obj_id = msg_obj.obj_id;
    rx_msg.timeout = timeout;
    rx_msg.timepoint = emb::chrono::steady_clock::now();
    rx_msg.unhandled = false;
    rx_msg.len = len;
    rx_msg.handler = handler;
    rx_msgs_.push_back(rx_msg);
}

void Node::register_tx_message(canid_t id,
                               uint16_t len,
                               emb::chrono::milliseconds period,
                               canpayload_t (*creator)()) {
    assert(!server_.message_objects_.full());

    mcu::c28x::can::MessageObject msg_obj;
    msg_obj.obj_id = server_.message_objects_.size();
    msg_obj.frame_id = id;
    if (id <= 0x7FF) {
        msg_obj.frame_type = CAN_MSG_FRAME_STD;
    } else {
        msg_obj.frame_type = CAN_MSG_FRAME_EXT;
    }
    msg_obj.obj_type = CAN_MSG_OBJ_TYPE_TX;
    msg_obj.frame_idmask = 0;
    msg_obj.flags = CAN_MSG_OBJ_NO_FLAGS;
    msg_obj.data_len = len;
    server_.message_objects_.push_back(msg_obj);
    server_.can_module_.setup_message_object(msg_obj);

    TxMessage tx_msg;
    tx_msg.obj_id = msg_obj.obj_id;
    tx_msg.period = period;
    tx_msg.timepoint = emb::chrono::steady_clock::now();
    tx_msg.len = len;
    tx_msg.creator = creator;
    tx_msgs_.push_back(tx_msg);
}

void Node::recv(uint32_t obj_id) {
    size_t idx;
    for (size_t i = 0; i < rx_msgs_.size(); ++i) {
        if (obj_id == rx_msgs_[i].obj_id) {
            if (rx_msgs_[i].unhandled) {
                return;
            } else {
                rx_msgs_[i].timepoint = emb::chrono::steady_clock::now();
                server_.can_module_.recv(obj_id, rx_msgs_[i].payload.data);
                rx_msgs_[i].unhandled = true;
                return;
            }
        }
    }
}

void Node::send() {
    emb::chrono::milliseconds now = emb::chrono::steady_clock::now();

    for (size_t i = 0; i < tx_msgs_.size(); ++i) {
        if (!tx_msgs_[i].creator || tx_msgs_[i].period.count() <= 0) {
            continue;
        }

        if (now < tx_msgs_[i].timepoint + tx_msgs_[i].period) {
            continue;
        }

        const canpayload_t payload = tx_msgs_[i].creator();
        server_.can_module_.send(tx_msgs_[i].obj_id,
                                 payload.data,
                                 tx_msgs_[i].len);
        tx_msgs_[i].timepoint = now;
    }
}

void Node::handle() {
    for (size_t i = 0; i < rx_msgs_.size(); ++i) {
        if (rx_msgs_[i].unhandled && rx_msgs_[i].handler != NULL) {
            rx_msgs_[i].handler(rx_msgs_[i].payload);
            rx_msgs_[i].unhandled = false;
        }
    }
}

bool Node::good() const {
    emb::chrono::milliseconds now = emb::chrono::steady_clock::now();
    for (size_t i = 0; i < rx_msgs_.size(); ++i) {
        if (now > rx_msgs_[i].timepoint + rx_msgs_[i].timeout) {
            return false;
        }
    }
    return true;
}

} // namespace ucanopen

#endif
