#pragma once

#ifdef MCUDRV_C28X

#include <ucanopen/c28x/server/impl/impl_server.hpp>
#include <emblib/chrono.hpp>

namespace ucanopen {

class TpdoService {
private:
    impl::Server& server_;

    struct Message {
        emb::chrono::milliseconds period;
        emb::chrono::milliseconds timepoint;
        canpayload_t (*creator)();
    };
    emb::array<Message, 4> tpdo_msgs_;
public:
    TpdoService(impl::Server& server);
    void register_tpdo(CobTpdo tpdo,
                       emb::chrono::milliseconds period,
                       canpayload_t (*creator)());
    void send();
};

} // namespace ucanopen

#endif
