#pragma once


#ifdef STM32H7xx


#include "../impl/impl_server.h"
#include <mcudrv/stm32/h7/chrono/chrono.h>


namespace ucanopen {

class HeartbeatService {
private:
    impl::Server& _server;
    std::chrono::milliseconds _period;
    std::chrono::milliseconds _timepoint;
    FDCAN_TxHeaderTypeDef _header;
public:
    HeartbeatService(impl::Server& server, std::chrono::milliseconds period);
    void send();
};

} // namespace ucanopen

#endif
