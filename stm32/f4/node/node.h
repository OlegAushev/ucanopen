#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32F4xx


#include "../server/impl/impl_server.h"
#include <chrono>
#include <vector>


namespace ucanopen {


class Server;


class Node : public impl::FrameReceiverInterface {
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
        CAN_TxHeaderTypeDef header;
        can_payload (*creator)();
    };
    std::vector<TxMessage> _tx_messages;
public:
    Node(Server& server);

    void register_rx_message(CAN_FilterTypeDef& filter, std::chrono::milliseconds timeout, void(*handler)(const can_payload&));
    void register_tx_message(const CAN_TxHeaderTypeDef& header, std::chrono::milliseconds period, can_payload (*creator)());

    virtual std::vector<mcu::can::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::RxMessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;

    void send();

    bool connection_ok();
};


} // namespace ucanopen


#endif
#endif
