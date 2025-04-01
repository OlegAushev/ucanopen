#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>

#include <emblib/chrono.hpp>

namespace ucanopen {

class TpdoService {
private:
    impl::Server& server_;

    struct Message {
        canid_t id;
        static constexpr uint8_t len = 8;
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        canpayload_t (*creator)();
    };
    std::array<Message, 4> messages_;
public:
    TpdoService(impl::Server& server);
    void register_tpdo(CobTpdo tpdo,
                       std::chrono::milliseconds period,
                       canpayload_t (*creator)());
    void send();
};

} // namespace ucanopen

#endif
#endif
