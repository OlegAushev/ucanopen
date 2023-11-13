#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <mcudrv/stm32/f4/chrono/chrono.h>


namespace ucanopen {


class HeartbeatService {
private:
    impl::Server& _server;
    can_id _id;
    static constexpr uint8_t _len = cob_sizes[std::to_underlying(Cob::heartbeat)];
    std::chrono::milliseconds _period;
    std::chrono::milliseconds _timepoint;
public:
    HeartbeatService(impl::Server& server, std::chrono::milliseconds period);
    void send();
};


} // namespace ucanopen


#endif
#endif
