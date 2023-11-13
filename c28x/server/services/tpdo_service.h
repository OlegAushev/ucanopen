#pragma once


#ifdef MCUDRV_C28X


#include "../impl/impl_server.h"
#include <mcudrv/c28x/f2837xd/chrono/chrono.h>


namespace ucanopen {


class TpdoService {
private:
    impl::Server& _server;

    struct Message {
        emb::chrono::milliseconds period;
        emb::chrono::milliseconds timepoint;
        can_payload (*creator)();
    };
    emb::array<Message, 4> _tpdo_msgs;
public:
    TpdoService(impl::Server& server);
    void register_tpdo(CobTpdo tpdo, emb::chrono::milliseconds period, can_payload (*creator)());
    void send();
};


} // namespace ucanopen


#endif
