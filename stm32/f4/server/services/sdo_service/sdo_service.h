#pragma once


#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include <ucanopen/stm32/f4/server/impl/impl_server.h>
#include <emblib/algorithm.hpp>
#include <emblib/queue.hpp>


namespace ucanopen {


class SdoService : public impl::FrameReceiver {
private:
    impl::Server& _server;

    ucan::RxMessageAttribute _rsdo_rxattr;
    emb::queue<can_payload, 16> _rsdo_queue;

    can_id _tsdo_id;
    static constexpr uint8_t _tsdo_len = cob_sizes[std::to_underlying(Cob::tsdo)];
    emb::queue<can_payload, 16> _tsdo_queue;
public:
    SdoService(impl::Server& server);
    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const override;
    virtual FrameRecvStatus recv_frame(const ucan::RxMessageAttribute& attr, const can_frame& frame) override;
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
