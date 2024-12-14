#ifdef MCUDRV_C28X

#include "server.h"

namespace ucanopen {

Server::Server(mcu::c28x::can::Module& can_module,
               const ServerConfig& config,
               ODEntry* object_dictionary,
               size_t object_dictionary_size)
        : impl::Server(can_module,
                       NodeId(config.node_id),
                       object_dictionary,
                       object_dictionary_size)
        , emb::interrupt_invoker_array<Server, mcu::c28x::can::peripheral_count>(
                this,
                can_module.peripheral().underlying_value()) {
    heartbeat_service =
            new HeartbeatService(*this,
                                 emb::chrono::milliseconds(
                                         config.heartbeat_period_ms));
    tpdo_service = new TpdoService(*this);
    rpdo_service = new RpdoService(*this);
    sdo_service = new SdoService(*this);

    node_map_.fill(NULL);

    this->can_module_.register_interrupt_callback(on_frame_received);

    this->nmt_state_ = NmtState::pre_operational;
}

void Server::run() {
    heartbeat_service->send();
    tpdo_service->send();
    sdo_service->send();
    rpdo_service->handle();
    sdo_service->handle();

    for (size_t i = 0; i < nodes_.size(); ++i) {
        nodes_[i]->send();
        nodes_[i]->handle();
    }

    on_run();
}


void Server::on_frame_received(mcu::c28x::can::Module* can_module,
                               uint32_t interrupt_cause,
                               uint16_t status) {
    Server* server =
            Server::instance(can_module->peripheral().underlying_value());

    switch (interrupt_cause) {
    case CAN_INT_INT0ID_STATUS:
        switch (status) {
        case CAN_STATUS_PERR:
        case CAN_STATUS_BUS_OFF:
        case CAN_STATUS_EWARN:
        case CAN_STATUS_LEC_BIT1:
        case CAN_STATUS_LEC_BIT0:
        case CAN_STATUS_LEC_CRC:
            server->on_can_bus_error();
            break;
        default:
            break;
        }
        break;
    case Cob::rpdo1:
    case Cob::rpdo2:
    case Cob::rpdo3:
    case Cob::rpdo4:
        server->rpdo_service->recv(interrupt_cause);
        break;
    case Cob::rsdo:
        server->sdo_service->recv(interrupt_cause);
        break;
    default:
        if ((interrupt_cause - cob_count) < server->node_map_.size()){
            const size_t idx = interrupt_cause - cob_count;
            if (server->node_map_[idx] != NULL) {
                server->node_map_[idx]->recv(interrupt_cause);
            }
        }
        break;
    }
}

} // namespace ucanopen

#endif
