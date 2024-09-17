#pragma once


#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include "../impl/impl_server.h"
#include <emblib/algorithm.h>


namespace ucanopen {


class SdoService : public impl::FrameReceiver {
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


template <typename T, size_t Size>
class SdoProvider {
private:
    static inline uint32_t _dummy_data = 42;
protected:
    SdoProvider() {
        std::fill(std::begin(sdo_data), std::end(sdo_data), &_dummy_data);
    }

    template <typename V>
    void _register_sdo_data(size_t idx, V& dataobj) {
        sdo_data[idx] = reinterpret_cast<uint32_t*>(&dataobj);
    }
public:
    static inline uint32_t* sdo_data[Size];
    size_t capacity() const { return Size; }
};


} // namespace ucanopen


#endif
#endif
