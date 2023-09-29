#pragma once


#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <mculib_stm32/f4/chrono/chrono.h>


namespace ucanopen {

class RpdoService : public impl::FrameReceiverInterface {
private:
    impl::Server& _server;

    struct Message {
        mcu::can::MessageAttribute attr;
        std::chrono::milliseconds timeout;
        std::chrono::milliseconds timepoint;
        bool is_unhandled;
        can_frame frame;
        void(*handler)(const can_payload&);
    };
    std::array<Message, 4> _rpdo_msgs;
public:
    RpdoService(impl::Server& server);
    
    void register_rpdo(CobRpdo rpdo, std::chrono::milliseconds timeout, void(*handler)(const can_payload&), can_id id = 0);
    
    virtual std::vector<mcu::can::MessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;

    bool is_ok(CobRpdo rpdo) {
        if (_rpdo_msgs[std::to_underlying(rpdo)].timeout.count() <= 0) {
            return true;
        }
        if (mcu::chrono::system_clock::now() <= _rpdo_msgs[std::to_underlying(rpdo)].timepoint + _rpdo_msgs[std::to_underlying(rpdo)].timeout) {
            return true;
        }
        return false;
    }
};

} // namespace ucanopen

#endif
