#pragma once


#ifdef STM32H7xx


#include "../impl/impl_server.h"
#include <mculib_stm32/h7/chrono/chrono.h>
#include <emblib_stm32/algorithm.h>


namespace ucanopen {

class SdoService : public impl::FrameReceiverInterface {
private:
    impl::Server& _server;

    struct RxMessage {
        mcu::can::MessageAttribute attr;
        bool is_unhandled;
        can_frame frame;
    } _rsdo;

    struct TxMessage {
        FDCAN_TxHeaderTypeDef header;
        bool not_sent;
        can_payload payload;
    } _tsdo;
public:
    SdoService(impl::Server& server);
    virtual std::vector<mcu::can::MessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;
    void send();
private:
    SdoAbortCode _read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
};

} // namespace ucanopen

#endif
