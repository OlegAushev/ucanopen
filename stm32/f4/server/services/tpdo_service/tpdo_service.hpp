#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>

#include <emblib/chrono.hpp>

namespace ucanopen {

class TpdoService {
private:
    impl::Server& _server;

    struct Message {
        can_id id;
        static constexpr uint8_t len = 8;
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        can_payload (*creator)();
    };
    std::array<Message, 4> _tpdo_msgs;
public:
    TpdoService(impl::Server& server);
    void register_tpdo(CobTpdo tpdo,
                       std::chrono::milliseconds period,
                       can_payload (*creator)());
    void send();
};

} // namespace ucanopen

#endif
#endif
