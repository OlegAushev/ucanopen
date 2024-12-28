#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>

#include <chrono>
#include <vector>

namespace ucanopen {

class Server;

class Node : public impl::FrameReceiver {
private:
    ucan::Module& can_module_;

    struct RxMessage {
        ucan::RxMessageAttribute attr;
        std::chrono::milliseconds timeout;
        std::chrono::milliseconds timepoint;
        bool unhandled;
        can_frame frame;
        void (*handler)(const can_payload&);
    };
    std::vector<RxMessage> rx_messages_;

    struct TxMessage {
        std::chrono::milliseconds period;
        std::chrono::milliseconds timepoint;
        can_id id;
        uint8_t len;
        can_payload (*creator)();
    };
    std::vector<TxMessage> tx_messages_;
public:
    Node(Server& server);
#if defined(MCUDRV_STM32)
    void register_rx_message(CAN_FilterTypeDef& filter,
                             std::chrono::milliseconds timeout,
                             void (*handler)(const can_payload&));
#elif defined(MCUDRV_APM32)
    void register_rx_message(CAN_FilterConfig_T& filter,
                             std::chrono::milliseconds timeout,
                             void (*handler)(const can_payload&));
#endif
    void register_tx_message(can_id id,
                             uint8_t len,
                             std::chrono::milliseconds period,
                             can_payload (*creator)());

    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const override;
    virtual void recv(const ucan::RxMessageAttribute& attr,
                      const can_frame& frame) override;
    virtual void handle() override;

    void send();

    virtual void inspect() {}
    bool good();
};

} // namespace ucanopen

#endif
#endif
