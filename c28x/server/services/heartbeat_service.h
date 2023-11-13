#pragma once


#include <mcudrv/c28x/f2837xd/chrono/chrono.h>
#include "../impl/impl_server.h"


namespace ucanopen {

class HeartbeatService {
private:
    impl::Server& _server;
    emb::chrono::milliseconds _period;
    emb::chrono::milliseconds _timepoint;
public:
    HeartbeatService(impl::Server& server, emb::chrono::milliseconds period);
    void send();
};

} // namespace ucanopen

