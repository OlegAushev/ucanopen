#include "rpdo_service.h"


namespace ucanopen {

unsigned char RpdoService::cana_rpdo_dualcore_alloc[sizeof(emb::array<RpdoService::Message, 4>)]
        __attribute__((section("shared_ucanopen_cana_rpdo_data"), retain));
unsigned char RpdoService::canb_rpdo_dualcore_alloc[sizeof(emb::array<RpdoService::Message, 4>)]
        __attribute__((section("shared_ucanopen_canb_rpdo_data"), retain));


RpdoService::RpdoService(impl::Server& server, const IpcFlags& ipc_flags)
        : _server(server) {
    switch (_server._ipc_mode.underlying_value()) {
    case mcu::ipc::Mode::singlecore:
        _rpdo_msgs = new emb::array<Message, 4>;
        break;
    case mcu::ipc::Mode::dualcore:
        switch (_server._can_peripheral.native_value()) {
        case mcu::can::Peripheral::cana:
            _rpdo_msgs = new(cana_rpdo_dualcore_alloc) emb::array<Message, 4>;
            break;
        case mcu::can::Peripheral::canb:
            _rpdo_msgs = new(canb_rpdo_dualcore_alloc) emb::array<Message, 4>;
            break;
        }
        break;
    }

    for (int i = 0; i < _rpdo_msgs->size(); ++i) {
        (*_rpdo_msgs)[i].timeout = emb::chrono::milliseconds(-1);
        (*_rpdo_msgs)[i].timepoint = mcu::chrono::system_clock::now();
    }

    for (int i = 0; i < _handlers.size(); ++i) {
        _handlers[i] = reinterpret_cast<void(*)(const can_payload& data)>(NULL);
    }

    _received_flags[CobRpdo::rpdo1] = ipc_flags.rpdo1_received;
    _received_flags[CobRpdo::rpdo2] = ipc_flags.rpdo2_received;
    _received_flags[CobRpdo::rpdo3] = ipc_flags.rpdo3_received;
    _received_flags[CobRpdo::rpdo4] = ipc_flags.rpdo4_received;
}


void RpdoService::register_rpdo(CobRpdo rpdo, emb::chrono::milliseconds timeout, unsigned int id) {
    assert(_server._ipc_role == mcu::ipc::Role::primary);

    (*_rpdo_msgs)[rpdo.underlying_value()].timeout = timeout;
    if (id != 0) {
        Cob cob = to_cob(rpdo);
        _server._message_objects[cob.underlying_value()].frame_id = id;
        _server._can_module->setup_message_object(_server._message_objects[cob.underlying_value()]);
    }
}


void RpdoService::register_rpdo_handler(CobRpdo rpdo, void (*handler)(const can_payload& data)) {
    assert(_server._ipc_mode == mcu::ipc::Mode::singlecore || _server._ipc_role == mcu::ipc::Role::secondary);

    _handlers[rpdo.underlying_value()] = handler;
}

} // namespace ucanopen

