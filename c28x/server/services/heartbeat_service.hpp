#pragma once

#ifdef MCUDRV_C28X

#include <ucanopen/c28x/server/impl/impl_server.hpp>
#include <emblib/chrono.hpp>

namespace ucanopen {

class HeartbeatService {
private:
    impl::Server& server_;
    emb::chrono::milliseconds period_;
    emb::chrono::milliseconds timepoint_;
public:
    HeartbeatService(impl::Server& server, emb::chrono::milliseconds period);
    void send();
};

} // namespace ucanopen

#endif
