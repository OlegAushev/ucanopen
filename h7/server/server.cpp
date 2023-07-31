#include "server.h"


namespace ucanopen {

Server::Server(mcu::can::Module& can_module, const ServerConfig& config,
               ODEntry* object_dictionary, int object_dictionary_size)
        : impl::Server(can_module, NodeId(config.node_id), object_dictionary, object_dictionary_size)
        , emb::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, std::to_underlying(can_module.peripheral()))
{
    heartbeat_service = new HeartbeatService(*this, std::chrono::milliseconds(config.heartbeat_period_ms));
    sync_service = new SyncService(*this, std::chrono::milliseconds(config.sync_period_ms));
    tpdo_service = new TpdoService(*this);
    rpdo_service = new RpdoService(*this);
    sdo_service = new SdoService(*this);

    nodes.reserve(16);
    _attr_map.reserve(64);

    _can_module.register_on_fifo0_frame_received_callback(on_frame_received);
    _can_module.register_on_fifo1_frame_received_callback(on_frame_received);

    _nmt_state = NmtState::pre_operational;
}


void Server::add_node(Node* node_) {
    nodes.push_back(node_);
}


void Server::start() {
    if (_attr_map.empty()) {
        for (auto attr : rpdo_service->get_rx_attr()) {
            _attr_map.push_back(std::make_pair(attr, rpdo_service));
        }

        for (auto attr : sdo_service->get_rx_attr()) {
            _attr_map.push_back(std::make_pair(attr, sdo_service));
        }

        for (const auto& node : nodes) {
            for (auto attr : node->get_rx_attr()) {
                _attr_map.push_back(std::make_pair(attr, node));
            }
        }
    }

    _can_module.start();
    _can_module.enable_interrupts();
    _nmt_state = NmtState::operational;
}


void Server::stop() {
    _can_module.stop();
    _can_module.disable_interrupts();
    _nmt_state = NmtState::stopped;
}


void Server::run() {
    heartbeat_service->send();
    sync_service->send();
    tpdo_service->send();
    sdo_service->send();
    for (auto node : nodes) {
        node->send();
    }

    rpdo_service->handle_recv_frames();
    sdo_service->handle_recv_frames();
    for (auto node : nodes) {
        node->handle_recv_frames();
    }

    on_run();
}


void Server::check_connection() {
    rpdo_service->connection_ok() ? _connection_status.set(0) : _connection_status.reset(0);
    for (auto i = 0; i < nodes.size(); ++i) {
        nodes[i]->connection_ok() ? _connection_status.set(i+1) : _connection_status.reset(i+1);
    }

    if (!_connection_status.all()) {
        syslog::set_warning(sys::Warning::can_bus_connection_lost);
    } else {
        syslog::reset_warning(sys::Warning::can_bus_connection_lost);
    }
}


void Server::on_frame_received(mcu::can::Module& can_module, const mcu::can::MessageAttribute& attr, const can_frame& frame) {
    auto server = Server::instance(std::to_underlying(can_module.peripheral()));
    auto receiver = std::find_if(server->_attr_map.begin(), server->_attr_map.end(),
                                 [attr](const auto& item){ return item.first == attr; });
    if (receiver != server->_attr_map.end()) {
        auto status = receiver->second->recv_frame(attr, frame);
        if (status != FrameRecvStatus::success) {
            ++server->_errcount;
        }
    }
}

}

