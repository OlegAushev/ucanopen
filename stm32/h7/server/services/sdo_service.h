#pragma once

#ifdef MCUDRV_STM32
#ifdef STM32H7xx

#include "../impl/impl_server.h"
#include <emblib/algorithm.hpp>
#include <mcudrv/stm32/h7/chrono/chrono.h>

namespace ucanopen {

class SdoService : public impl::FrameReceiver {
private:
    impl::Server& _server;

    struct RxMessage {
        ucan::RxMessageAttribute attr;
        bool is_unhandled;
        can_frame frame;
    } _rsdo;

    struct TxMessage {
        FDCAN_TxHeaderTypeDef header;
        bool not_sent;
        canpayload_t payload;
    } _tsdo;
public:
    SdoService(impl::Server& server);
    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const ucan::RxMessageAttribute& attr,
                                       const can_frame& frame) override;
    virtual void handle_recv_frames() override;
    void send();
private:
    SdoAbortCode _read_expedited(const ODEntry* od_entry,
                                 ExpeditedSdo& tsdo,
                                 const ExpeditedSdo& rsdo);
    SdoAbortCode _write_expedited(const ODEntry* od_entry,
                                  ExpeditedSdo& tsdo,
                                  const ExpeditedSdo& rsdo);
    SdoAbortCode _restore_default_parameter(ODObjectKey key);

    static const ODObjectKey restore_default_parameter_key;
};

} // namespace ucanopen

#endif
#endif
