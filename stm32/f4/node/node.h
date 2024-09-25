#pragma once


#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include <ucanopen/stm32/f4/server/impl/impl_server.h>
#include <chrono>
#include <vector>


namespace ucanopen {


class Server;


class Node : public impl::FrameReceiver {
private:
    mcu::can::Module& _can_module;

    struct RxMessage {
        mcu::can::RxMessageAttribute attr;
        std::chrono::milliseconds timeout;
        std::chrono::milliseconds timepoint;
        bool is_unhandled;
        can_frame frame;
        void(*handler)(const can_payload&);
    };
    std::vector<RxMessage> _rx_messages;

    struct TxMessage {
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        can_id id;
        uint8_t len;
        can_payload (*creator)();
    };
    std::vector<TxMessage> _tx_messages;
public:
    Node(Server& server);
#if defined(MCUDRV_STM32)
    void register_rx_message(CAN_FilterTypeDef& filter, std::chrono::milliseconds timeout, void(*handler)(const can_payload&));
#elif defined(MCUDRV_APM32)
    void register_rx_message(CAN_FilterConfig_T& filter, std::chrono::milliseconds timeout, void(*handler)(const can_payload&));
#endif
    void register_tx_message(can_id id, uint8_t len, std::chrono::milliseconds period, can_payload (*creator)());

    virtual std::vector<mcu::can::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::RxMessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;

    void send();

    bool connection_ok();
};


} // namespace ucanopen


#endif
#endif
