#pragma once


#include "../impl/impl_server.h"
#include <mculib_c28x/f2837xd/chrono/chrono.h>
#include <new>


namespace ucanopen {

class RpdoService {
private:
    impl::Server& _server;

    struct Message {
        emb::chrono::milliseconds timeout;
        emb::chrono::milliseconds timepoint;
        can_payload payload;
    };
    emb::array<Message, 4>* _rpdo_msgs;
    static unsigned char cana_rpdo_dualcore_alloc[sizeof(emb::array<Message, 4>)];
    static unsigned char canb_rpdo_dualcore_alloc[sizeof(emb::array<Message, 4>)];
    emb::array<mcu::ipc::Flag, 4> _received_flags;
    emb::array<void(*)(const can_payload& payload), 4> _handlers;
public:
    RpdoService(impl::Server& server, const IpcFlags& ipc_flags);
    void register_rpdo(CobRpdo rpdo, emb::chrono::milliseconds timeout, unsigned int id = 0);
    void register_rpdo_handler(CobRpdo rpdo, void (*handler)(const can_payload& data));

    void recv(Cob cob) {
        assert(_server._ipc_role == mcu::ipc::Role::primary);

        if (cob != Cob::rpdo1
         && cob != Cob::rpdo2
         && cob != Cob::rpdo3
         && cob != Cob::rpdo4) { return; }

        CobRpdo rpdo((cob.underlying_value() - static_cast<unsigned int>(Cob::rpdo1)) / 2);

        (*_rpdo_msgs)[rpdo.underlying_value()].timepoint = mcu::chrono::system_clock::now();
        if (_received_flags[rpdo.underlying_value()].local.is_set()) {
            _server.on_rpdo_overrun();
        } else {
            // there is no unprocessed RPDO of this type
            _server._can_module->recv(cob.underlying_value(), (*_rpdo_msgs)[rpdo.underlying_value()].payload.data);
            _received_flags[rpdo.underlying_value()].local.set();
        }
    }

    void handle_received() {
        assert(_server._ipc_mode == mcu::ipc::Mode::singlecore || _server._ipc_role == mcu::ipc::Role::secondary);

        for (size_t i = 0; i < _rpdo_msgs->size(); ++i) {
            if (!_handlers[i]) { continue; }
            if (_received_flags[i].is_set()) {
                _handlers[i]((*_rpdo_msgs)[i].payload);
                _received_flags[i].reset();
            }
        }
    }

    bool is_ok(CobRpdo rpdo) {
        if ((*_rpdo_msgs)[rpdo.underlying_value()].timeout.count() <= 0) {
            return true;
        }
        if (mcu::chrono::system_clock::now() <= (*_rpdo_msgs)[rpdo.underlying_value()].timepoint + (*_rpdo_msgs)[rpdo.underlying_value()].timeout) {
            return true;
        }
        return false;
    }
};

} // namespace ucanopen

