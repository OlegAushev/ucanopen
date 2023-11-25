#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include "sync_service.h"


namespace ucanopen {


SyncService::SyncService(impl::Server& server, std::chrono::milliseconds period)
        : _server(server)
        , _period(period) {
    _id = calculate_cob_id(Cob::sync, _server.node_id());
    _timepoint = mcu::chrono::system_clock::now();
}


void SyncService::send() {
    if (_period.count() <= 0) { return; }

    auto now = mcu::chrono::system_clock::now();
    if (now >= _timepoint + _period) {
        can_payload payload = {};
        _server._can_module.send({_id, _len, payload});
        _timepoint = now;
    }
}
    

} // namespace ucanopen


#endif
#endif
