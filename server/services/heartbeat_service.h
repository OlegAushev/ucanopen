#pragma once


#include <mculib_c28x/f2837xd/chrono/chrono.h>
#include "../impl/impl_server.h"


namespace ucanopen {

class HeartbeatService {
private:
    impl::Server& _server;
    emb::chrono::milliseconds _period;
    emb::chrono::milliseconds _timepoint;
public:
    HeartbeatService(impl::Server& server, emb::chrono::milliseconds period);

    void send() {
        assert(_server._ipc_role == mcu::ipc::Role::primary);

        if (_period.count() <= 0)  { return; }

        emb::chrono::milliseconds now = mcu::chrono::system_clock::now();
        if (now >= _timepoint + _period) {
            can_payload payload;
            payload[0] = _server.nmt_state().underlying_value();
            _server._can_module->send(CobType::heartbeat, payload.data, cob_sizes[CobType::heartbeat]);
            _timepoint = now;
        }
    }
};

} // namespace ucanopen

