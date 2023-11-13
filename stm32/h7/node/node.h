#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32H7xx


#include "../server/impl/impl_server.h"
#include <mcudrv/stm32/h7/chrono/chrono.h>
#include <chrono>
#include <functional>
#include <vector>


namespace ucanopen {


class Server;


class Node : public impl::FrameReceiverInterface {
private:
    mcu::can::Module& _can_module;

    struct RxMessage {
        mcu::can::MessageAttribute attr;
        std::chrono::milliseconds timeout;
        std::chrono::milliseconds timepoint;
        bool is_unhandled;
        can_frame frame;
        std::function<void(const can_payload&)> handler;
        //void(*handler)(const can_payload&);
    };
    std::vector<RxMessage> _rx_messages;

    struct TxMessage {
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        FDCAN_TxHeaderTypeDef header;
        std::function<can_payload(void)> creator;
        //can_payload (*creator)();
    };
    std::vector<TxMessage> _tx_messages;

    bool _enabled = true;
public:
    Node(Server& server);

    void register_rx_message(FDCAN_FilterTypeDef& filter, std::chrono::milliseconds timeout, std::function<void(const can_payload&)> handler);
    void register_tx_message(const FDCAN_TxHeaderTypeDef& header, std::chrono::milliseconds period, std::function<can_payload(void)> creator);

    virtual std::vector<mcu::can::MessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;

    void send();

    bool connection_ok();

    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
};


} // namespace ucanopen


#endif
#endif
