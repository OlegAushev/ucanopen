#include "tpdo_service.h"


namespace ucanopen {

TpdoService::TpdoService(impl::Server& server)
        : _server(server) {
    for (size_t i = 0; i < _tpdo_msgs.size(); ++i) {
        _tpdo_msgs[i].period = emb::chrono::milliseconds(-1);
        _tpdo_msgs[i].timepoint = mcu::chrono::system_clock::now();
        _tpdo_msgs[i].creator = reinterpret_cast<can_payload(*)()>(NULL);
    }
}


void TpdoService::register_tpdo(CobTpdo tpdo, emb::chrono::milliseconds period, can_payload (*creator)()) {
    assert(_server._ipc_role == mcu::ipc::Role::primary);

    _tpdo_msgs[tpdo.underlying_value()].period = period;
    _tpdo_msgs[tpdo.underlying_value()].creator = creator;
}

} // namespace ucanopen

