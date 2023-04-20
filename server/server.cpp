#include "server.h"


namespace ucanopen {

Server::Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
               mcu::can::Module* can_module, const ServerConfig& config,
               ODEntry* object_dictionary, int object_dictionary_size)
        : impl::Server(mcu::ipc::traits::singlecore(), mcu::ipc::traits::primary(),
                       can_module, NodeId(config.node_id), object_dictionary, object_dictionary_size)
        , emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, can_module->peripheral().underlying_value()) {
    assert(ipc_flags.rpdo1_received.mode() == mcu::ipc::Mode::singlecore);
    assert(ipc_flags.rpdo2_received.mode() == mcu::ipc::Mode::singlecore);
    assert(ipc_flags.rpdo3_received.mode() == mcu::ipc::Mode::singlecore);
    assert(ipc_flags.rpdo4_received.mode() == mcu::ipc::Mode::singlecore);
    assert(ipc_flags.rsdo_received.mode() == mcu::ipc::Mode::singlecore);
    assert(ipc_flags.tsdo_ready.mode() == mcu::ipc::Mode::singlecore);

    heartbeat_service = new HeartbeatService(this, emb::chrono::milliseconds(config.heartbeat_period_ms));
    tpdo_service = new TpdoService(this);
    rpdo_service = new RpdoService(this, ipc_flags);
    sdo_service = new SdoService(this, ipc_flags);

    this->_can_module->register_interrupt_callback(on_frame_received);

    this->_nmt_state = NmtState::pre_operational;
}


Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
               mcu::can::Module* can_module, const ServerConfig& config)
        : impl::Server(mcu::ipc::traits::dualcore(), mcu::ipc::traits::primary(), can_module, NodeId(config.node_id))
        , emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, can_module->peripheral().underlying_value()) {
    assert(ipc_flags.rpdo1_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rpdo2_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rpdo3_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rpdo4_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rsdo_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.tsdo_ready.mode() == mcu::ipc::Mode::dualcore);

    heartbeat_service = new HeartbeatService(this, emb::chrono::milliseconds(config.heartbeat_period_ms));
    tpdo_service = new TpdoService(this);
    rpdo_service = new RpdoService(this, ipc_flags);
    sdo_service = new SdoService(this, ipc_flags);

    this->_can_module->register_interrupt_callback(on_frame_received);

    this->_nmt_state = NmtState::pre_operational;
}


Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary, const IpcFlags& ipc_flags,
               mcu::can::Peripheral can_peripheral, ODEntry* object_dictionary, int object_dictionary_size)
        : impl::Server(mcu::ipc::traits::dualcore(), mcu::ipc::traits::secondary(), can_peripheral, object_dictionary, object_dictionary_size)
        , emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, can_peripheral.underlying_value()) {
    assert(ipc_flags.rpdo1_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rpdo2_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rpdo3_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rpdo4_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.rsdo_received.mode() == mcu::ipc::Mode::dualcore);
    assert(ipc_flags.tsdo_ready.mode() == mcu::ipc::Mode::dualcore);

    rpdo_service = new RpdoService(this, ipc_flags);
    sdo_service = new SdoService(this, ipc_flags);
}


void Server::on_frame_received(mcu::can::Module* can_module, uint32_t interrupt_cause, uint16_t status) {
    Server* server = Server::instance(can_module->peripheral().underlying_value());

    switch (interrupt_cause) {
    case CAN_INT_INT0ID_STATUS:
        switch (status) {
        case CAN_STATUS_PERR:
        case CAN_STATUS_BUS_OFF:
        case CAN_STATUS_EWARN:
        case CAN_STATUS_LEC_BIT1:
        case CAN_STATUS_LEC_BIT0:
        case CAN_STATUS_LEC_CRC:
            syslog::set_warning(sys::Warning::can_bus_error);
            break;
        default:
            break;
        }
        break;
    case CobType::rpdo1:
    case CobType::rpdo2:
    case CobType::rpdo3:
    case CobType::rpdo4:
        syslog::reset_warning(sys::Warning::can_bus_error);
        server->rpdo_service->recv(CobType(interrupt_cause));
        break;
    case CobType::rsdo:
        syslog::reset_warning(sys::Warning::can_bus_error);
        server->sdo_service->recv();
        break;
    default:
        break;
    }
}

} // namespace ucanopen

