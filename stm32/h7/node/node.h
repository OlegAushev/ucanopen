#pragma once

#ifdef MCUDRV_STM32
#ifdef STM32H7xx

#include "../server/impl/impl_server.h"
#include <chrono>
#include <functional>
#include <mcudrv/stm32/h7/chrono/chrono.h>
#include <vector>

namespace ucanopen {

class Server;

class Node : public impl::FrameReceiver {
private:
    ucan::Module& _can_module;

    struct RxMessage {
        ucan::RxMessageAttribute attr;
        std::chrono::milliseconds timeout;
        std::chrono::milliseconds timepoint;
        bool is_unhandled;
        can_frame frame;
        std::function<void(const canpayload_t&)> handler;
        //void(*handler)(const canpayload_t&);
    };
    std::vector<RxMessage> _rx_messages;

    struct TxMessage {
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        FDCAN_TxHeaderTypeDef header;
        std::function<canpayload_t(void)> creator;
        //canpayload_t (*creator)();
    };
    std::vector<TxMessage> _tx_messages;

    bool _enabled = true;
public:
    Node(Server& server);

    void register_rx_message(FDCAN_FilterTypeDef& filter,
                             std::chrono::milliseconds timeout,
                             std::function<void(const canpayload_t&)> handler);
    void register_tx_message(const FDCAN_TxHeaderTypeDef& header,
                             std::chrono::milliseconds period,
                             std::function<canpayload_t(void)> creator);

    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const ucan::RxMessageAttribute& attr,
                                       const can_frame& frame) override;
    virtual void handle_recv_frames() override;
    virtual void on_run() {}
    void send();
    bool good();
    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
};

} // namespace ucanopen

#endif
#endif
