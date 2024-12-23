#pragma once

#ifdef MCUDRV_C28X

#include <ucanopen/c28x/server/impl/impl_server.hpp>
#include <emblib/algorithm.hpp>
#include <emblib/queue.hpp>

namespace ucanopen {

class SdoService {
private:
    impl::Server& server_;

    emb::queue<can_payload, 16> rsdo_queue_;
    emb::queue<can_payload, 16> tsdo_queue_;
public:
    SdoService(impl::Server& server);
    void recv(uint32_t obj_id);
    void send();
    void handle();
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

template<typename T, size_t Size>
class SdoProvider {
private:
    static uint32_t dummy_data_;
protected:
    SdoProvider() {
        std::fill(sdo_data, sdo_data + Size, &dummy_data_);
    }

    template<typename V>
    void _register_sdo_data(size_t idx, V& dataobj) {
        sdo_data[idx] = reinterpret_cast<uint32_t*>(&dataobj);
    }
public:
    static uint32_t* sdo_data[Size];
    size_t capacity() const { return Size; }
};

template<typename T, size_t Size>
uint32_t SdoProvider<T, Size>::dummy_data_ = 42;
template<typename T, size_t Size>
uint32_t* SdoProvider<T, Size>::sdo_data[Size];

} // namespace ucanopen

#endif
