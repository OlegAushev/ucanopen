#pragma once


#ifdef STM32H7xx


#include "../impl/impl_server.h"
#include <mculib_stm32/h7/chrono/chrono.h>


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
    std::array<Message, 4> _rpdo_list;
public:
    RpdoService(impl::Server& server);
    
    void register_rpdo(RpdoType rpdo_type, std::chrono::milliseconds timeout, void(*handler)(const can_payload&), can_id id = 0);
    
    virtual std::vector<mcu::can::MessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;

    bool connection_ok();
};

} // namespace ucanopen

#endif
