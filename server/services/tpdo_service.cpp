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


void TpdoService::send() {
    assert(_server._ipc_role == mcu::ipc::Role::primary);

    emb::chrono::milliseconds now = mcu::chrono::system_clock::now();
    for (size_t i = 0; i < _tpdo_msgs.size(); ++i) {
        if (!_tpdo_msgs[i].creator || _tpdo_msgs[i].period.count() <= 0) { continue; }
        if (now < _tpdo_msgs[i].timepoint + _tpdo_msgs[i].period) { continue; }

        can_payload payload = _tpdo_msgs[i].creator();
        Cob cob = to_cob(CobTpdo(i));
        _server._can_module->send(cob.underlying_value(), payload.data, cob_data_len[cob.underlying_value()]);
        _tpdo_msgs[i].timepoint = now;
    }
}

} // namespace ucanopen

