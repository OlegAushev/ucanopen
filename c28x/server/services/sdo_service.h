#pragma once


#ifdef MCUDRV_C28X


#include "../impl/impl_server.h"
#include <emblib/algorithm.h>
#include <new>


namespace ucanopen {


class SdoService {
private:
    impl::Server& _server;

    mcu::c28x::ipc::Flag _rsdo_flag;
    mcu::c28x::ipc::Flag _tsdo_flag;
    can_payload* _rsdo_data;
    can_payload* _tsdo_data;
    static unsigned char cana_rsdo_dualcore_alloc[sizeof(can_payload)];
    static unsigned char canb_rsdo_dualcore_alloc[sizeof(can_payload)];
    static unsigned char cana_tsdo_dualcore_alloc[sizeof(can_payload)];
    static unsigned char canb_tsdo_dualcore_alloc[sizeof(can_payload)];
public:
    SdoService(impl::Server& server, const IpcFlags& ipc_flags);
    void recv();
    void send();
    void handle_received();
private:
    SdoAbortCode _read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _restore_default_parameter(ODObjectKey key);

    static const ODObjectKey restore_default_parameter_key;
};


template<typename T, size_t Size>
class SdoProvider {
private:
    static uint32_t _dummy_data;
protected:
    SdoProvider() {
        std::fill(sdo_data, sdo_data + Size, &_dummy_data);
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
uint32_t SdoProvider<T, Size>::_dummy_data = 42;
template<typename T, size_t Size>
uint32_t* SdoProvider<T, Size>::sdo_data[Size];


} // namespace ucanopen


#endif
