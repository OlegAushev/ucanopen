#ifdef MCUDRV_C28X


#include "heartbeat_service.h"


namespace ucanopen {
    

HeartbeatService::HeartbeatService(impl::Server& server, emb::chrono::milliseconds period)
        : _server(server)
        , _period(period) {
    _timepoint = mcu::c28x::chrono::steady_clock::now();
}


void HeartbeatService::send() {
    assert(_server._ipc_role == mcu::c28x::ipc::Role::primary);

    if (_period.count() <= 0)  { return; }

    emb::chrono::milliseconds now = mcu::c28x::chrono::steady_clock::now();
    if (now >= _timepoint + _period) {
        can_payload payload;
        payload[0] = _server.nmt_state().underlying_value();
        _server._can_module->send(Cob::heartbeat, payload.data, cob_data_len[Cob::heartbeat]);
        _timepoint = now;
    }
}


} // namespace ucanopen


#endif
