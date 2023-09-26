#pragma once


#include "../impl/impl_server.h"
#include <mculib_c28x/f2837xd/chrono/chrono.h>


namespace ucanopen {

class TpdoService {
private:
    impl::Server& _server;

    struct Message {
        emb::chrono::milliseconds period;
        emb::chrono::milliseconds timepoint;
        can_payload (*creator)();
    };
    emb::array<Message, 4> _tpdo_msgs;
public:
    TpdoService(impl::Server& server);
    void register_tpdo(CobTpdo tpdo, emb::chrono::milliseconds period, can_payload (*creator)());

    void send() {
        assert(_server._ipc_role == mcu::ipc::Role::primary);

        emb::chrono::milliseconds now = mcu::chrono::system_clock::now();
        for (int i = 0; i < _tpdo_msgs.size(); ++i) {
            if (!_tpdo_msgs[i].creator || _tpdo_msgs[i].period.count() <= 0) { continue; }
            if (now < _tpdo_msgs[i].timepoint + _tpdo_msgs[i].period) { continue; }

            can_payload payload = _tpdo_msgs[i].creator();
            Cob cob = to_cob(CobTpdo(i));
            _server._can_module->send(cob.underlying_value(), payload.data, cob_data_len[cob.underlying_value()]);
            _tpdo_msgs[i].timepoint = now;
        }
    }
};

} // namespace ucanopen

