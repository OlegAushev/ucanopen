#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32F4xx


#include "../impl/impl_server.h"
#include <emblib/algorithm.h>


namespace ucanopen {


class SdoService : public impl::FrameReceiverInterface {
private:
    impl::Server& _server;

    struct RxMessage {
        mcu::can::RxMessageAttribute attr;
        bool is_unhandled;
        can_frame frame;
    } _rsdo;

    struct TxMessage {
        can_id id;
        static constexpr uint8_t len = cob_sizes[std::to_underlying(Cob::tsdo)];
        bool not_sent;
        can_payload payload;
    } _tsdo;
public:
    SdoService(impl::Server& server);
    virtual std::vector<mcu::can::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const mcu::can::RxMessageAttribute& attr, const can_frame& frame) override;
    virtual void handle_recv_frames() override;
    void send();
private:
    SdoAbortCode _read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _restore_default_parameter(ODObjectKey key);

    static const ODObjectKey restore_default_parameter_key;
};


} // namespace ucanopen


#endif
#endif
