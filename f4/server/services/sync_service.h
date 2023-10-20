#pragma once


#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <mculib_stm32/f4/chrono/chrono.h>


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
