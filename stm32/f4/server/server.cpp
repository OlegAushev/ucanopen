#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/server.hpp>

namespace ucanopen {

Server::Server(ucan::Module& can_module,
               const ServerConfig& config,
               std::vector<ODEntry>& object_dictionary)
        : impl::Server(can_module,
                       NodeId(config.node_id),
                       object_dictionary),
          emb::singleton_array<Server, ucan::peripheral_count>(
                  this, std::to_underlying(can_module.peripheral())) {
    heartbeat_service = new HeartbeatService(
            *this, std::chrono::milliseconds(config.heartbeat_period_ms));
    sync_service = new SyncService(
            *this, std::chrono::milliseconds(config.sync_period_ms));
    tpdo_service = new TpdoService(*this);
    rpdo_service = new RpdoService(*this);
    sdo_service = new SdoService(*this);

    nodes.reserve(16);
    rxattr_map_.reserve(64);

#if defined(MCUDRV_STM32)
    can_module.init_interrupts(CAN_IT_RX_FIFO0_MSG_PENDING |
                               CAN_IT_RX_FIFO1_MSG_PENDING |
                               CAN_IT_TX_MAILBOX_EMPTY);
#elif defined(MCUDRV_APM32)
    can_module.init_interrupts(CAN_INT_F0MP | CAN_INT_F1MP | CAN_INT_TXME);
#endif

    nmt_state_ = NmtState::pre_operational;
}

void Server::add_node(Node* node_) { nodes.push_back(node_); }

void Server::start() {
    if (rxattr_map_.empty()) {
        for (auto attr : rpdo_service->get_rx_attr()) {
            rxattr_map_.push_back(std::make_pair(attr, rpdo_service));
        }

        for (auto attr : sdo_service->get_rx_attr()) {
            rxattr_map_.push_back(std::make_pair(attr, sdo_service));
        }

        for (const auto& node : nodes) {
            for (auto attr : node->get_rx_attr()) {
                rxattr_map_.push_back(std::make_pair(attr, node));
            }
        }
    }

    can_module_.start();
    can_module_.enable_interrupts();
    nmt_state_ = NmtState::operational;
}

void Server::stop() {
    can_module_.stop();
    can_module_.disable_interrupts();
    nmt_state_ = NmtState::stopped;
}

void Server::run() {
    heartbeat_service->send();
    sync_service->send();
    tpdo_service->send();
    sdo_service->send();
    rpdo_service->handle();
    sdo_service->handle();
    inspect();

    for (auto node : nodes) {
        node->send();
        node->handle();
        node->inspect();
    }
}

void Server::on_frame_received(ucan::Module& can_module,
                               const ucan::RxMessageAttribute& attr,
                               const can_frame& frame) {
    auto receiver = std::find_if(
            rxattr_map_.begin(), rxattr_map_.end(), [attr](const auto& item) {
                return item.first == attr;
            });
    if (receiver != rxattr_map_.end()) {
        receiver->second->recv(attr, frame);
    }
}

} // namespace ucanopen

#endif
#endif
