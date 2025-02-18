#pragma once

#ifdef MCUDRV_C28X

#include <ucanopen/c28x/server/impl/impl_server.hpp>
#include <emblib/chrono.hpp>

namespace ucanopen {

class RpdoService {
private:
    impl::Server& server_;

    struct Message {
        emb::chrono::milliseconds timeout;
        emb::chrono::milliseconds timepoint;
        bool unhandled;
        canpayload_t payload;
        void (*handler)(const canpayload_t&);
    };
    emb::array<Message, 4> rpdo_msgs_;
public:
    RpdoService(impl::Server& server);
    void register_rpdo(CobRpdo rpdo,
                       emb::chrono::milliseconds timeout,
                       void (*handler)(const canpayload_t&),
                       canid_t id = 0);
    void recv(uint32_t obj_id);
    void handle();

    bool good(CobRpdo rpdo) const {
        const size_t idx = rpdo.underlying_value();
        if (rpdo_msgs_[idx].timeout.count() <= 0) {
            return true;
        }
        if (emb::chrono::steady_clock::now() <=
            (rpdo_msgs_[idx].timepoint + rpdo_msgs_[idx].timeout)) {
            return true;
        }
        return false;
    }
};

} // namespace ucanopen

#endif
