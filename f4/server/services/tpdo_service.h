#pragma once


#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <mculib_stm32/f4/chrono/chrono.h>


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
    void register_tpdo(CobTpdo tpdo, std::chrono::milliseconds period, can_payload (*creator)());
    void send();
};

} // namespace ucanopen

#endif
