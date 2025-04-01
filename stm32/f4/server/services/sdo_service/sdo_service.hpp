#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>

#include <emblib/algorithm.hpp>
#include <emblib/queue.hpp>

namespace ucanopen {

class SdoService : public impl::FrameReceiver {
private:
    impl::Server& server_;

    ucan::RxMessageAttribute rsdo_rxattr_;
    emb::queue<canpayload_t, 16> rsdo_queue_;

    canid_t tsdo_id_;
    static constexpr uint8_t tsdo_len_ =
            cob_sizes[std::to_underlying(Cob::tsdo)];
    emb::queue<canpayload_t, 16> tsdo_queue_;
public:
    SdoService(impl::Server& server);
    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const override;
    virtual void recv(const ucan::RxMessageAttribute& attr,
                      const can_frame& frame) override;
    virtual void handle() override;
    void send();
private:
    SdoAbortCode read_expedited(const ODEntry* od_entry,
                                ExpeditedSdo& tsdo,
                                const ExpeditedSdo& rsdo);
    SdoAbortCode write_expedited(const ODEntry* od_entry,
                                 ExpeditedSdo& tsdo,
                                 const ExpeditedSdo& rsdo);
    SdoAbortCode restore_default_parameter(ODObjectKey key);

    static const ODObjectKey restore_default_parameter_key;
};

template<typename Derived, size_t DataCount>
class SdoProvider {
private:
    static inline uint32_t dummy_data_ = 42;
protected:
    SdoProvider() {
        std::fill(std::begin(sdo_data), std::end(sdo_data), &dummy_data_);
    }

    template<typename V>
    void register_sdo_data(size_t idx, V& dataobj) {
        sdo_data[idx] = reinterpret_cast<uint32_t*>(&dataobj);
    }
public:
    static inline uint32_t* sdo_data[DataCount];
    size_t capacity() const { return DataCount; }
};

} // namespace ucanopen

#endif
#endif
