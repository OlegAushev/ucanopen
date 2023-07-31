#pragma once


#ifdef STM32H7xx


#include "../impl/impl_server.h"
#include <mculib_stm32/h7/chrono/chrono.h>


namespace ucanopen {

class TpdoService {
private:
    impl::Server& _server;

    struct Message {
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        FDCAN_TxHeaderTypeDef header;
        can_payload (*creator)();
    };
    std::array<Message, 4> _tpdo_list;
public:
    TpdoService(impl::Server& server);
    void register_tpdo(TpdoType tpdo_type, std::chrono::milliseconds period, can_payload (*creator)());
    void send();
};

} // namespace ucanopen

#endif
