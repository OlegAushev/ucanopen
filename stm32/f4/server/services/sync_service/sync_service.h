#pragma once


#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include <ucanopen/stm32/f4/server/impl/impl_server.h>
#if defined(MCUDRV_STM32)
#include <mcudrv/stm32/f4/chrono/chrono.h>
#elif defined(MCUDRV_APM32)
#include <mcudrv/apm32/f4/chrono/chrono.h>
#endif


namespace ucanopen {


class SyncService {
private:
    impl::Server& _server;
    can_id _id;
    static constexpr uint8_t _len = cob_sizes[std::to_underlying(Cob::sync)];
    std::chrono::milliseconds _period;
    std::chrono::milliseconds _timepoint;
public:
    SyncService(impl::Server& server, std::chrono::milliseconds period);
    void send();
};


} // namespace ucanopen


#endif
#endif
