#include "tpdo_service.h"


namespace ucanopen {

TpdoService::TpdoService(impl::Server& server)
        : _server(server) {
    for (int i = 0; i < _tpdo_list.size(); ++i) {
        _tpdo_list[i].period = emb::chrono::milliseconds(-1);
        _tpdo_list[i].timepoint = mcu::chrono::system_clock::now();
        _tpdo_list[i].creator = reinterpret_cast<can_payload(*)()>(NULL);
    }
}


void TpdoService::register_tpdo(TpdoType tpdo_type, emb::chrono::milliseconds period, can_payload (*creator)()) {
    assert(_server._ipc_role == mcu::ipc::Role::primary);

    _tpdo_list[tpdo_type.underlying_value()].period = period;
    _tpdo_list[tpdo_type.underlying_value()].creator = creator;
}

} // namespace ucanopen

