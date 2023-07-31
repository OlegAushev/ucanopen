#pragma once


#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <mculib_stm32/f4/chrono/chrono.h>


namespace ucanopen {

class SyncService {
private:
    impl::Server& _server;
    std::chrono::milliseconds _period;
    std::chrono::milliseconds _timepoint;
    CAN_TxHeaderTypeDef _header;
public:
    SyncService(impl::Server& server, std::chrono::milliseconds period);
    void send();
};

} // namespace ucanopen

#endif
