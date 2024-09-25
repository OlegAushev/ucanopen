#pragma once


#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include <ucanopen/stm32/f4/server/impl/impl_server.h>
#if defined(MCUDRV_STM32)
#include <mcudrv/stm32/f4/chrono/chrono.h>
#elif defined(MCUDRV_APM32)
#include <mcudrv/apm32/f4/chrono/chrono.h>
#endif


namespace ucanopen {


class RpdoService : public impl::FrameReceiver {
private:
    impl::Server& _server;

    struct Message {
        mcu::can::RxMessageAttribute attr;
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
    
    virtual std::vector<mcu::can::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::RxMessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;

    bool good(CobRpdo rpdo) {
        if (_rpdo_msgs[std::to_underlying(rpdo)].timeout.count() <= 0) {
            return true;
        }
        if (mcu::chrono::steady_clock::now() <= _rpdo_msgs[std::to_underlying(rpdo)].timepoint + _rpdo_msgs[std::to_underlying(rpdo)].timeout) {
            return true;
        }
        return false;
    }
};


} // namespace ucanopen


#endif
#endif
