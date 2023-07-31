#pragma once


#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <mculib_stm32/f4/chrono/chrono.h>


namespace ucanopen {

class HeartbeatService {
private:
    impl::Server& _server;
    std::chrono::milliseconds _period;
    std::chrono::milliseconds _timepoint;
    CAN_TxHeaderTypeDef _header;
public:
    HeartbeatService(impl::Server& server, std::chrono::milliseconds period);
    void send();
};

} // namespace ucanopen

#endif
