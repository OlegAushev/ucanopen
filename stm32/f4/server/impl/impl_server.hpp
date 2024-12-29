#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/ucanopen_def.hpp>
#if defined(MCUDRV_STM32)
#include <mcudrv/stm32/f4/can/can.h>
namespace ucan = mcu::stm32::can;
#elif defined(MCUDRV_APM32)
#include <mcudrv/apm32/f4/can/can.hpp>
namespace ucan = mcu::apm32::can;
#endif
#include <algorithm>
#include <emblib/algorithm.hpp>
#include <vector>

namespace ucanopen {

class HeartbeatService;
class SyncService;
class TpdoService;
class RpdoService;
class SdoService;
class Node;

namespace impl {

class Server {
    friend class ucanopen::HeartbeatService;
    friend class ucanopen::SyncService;
    friend class ucanopen::TpdoService;
    friend class ucanopen::RpdoService;
    friend class ucanopen::SdoService;
    friend class ucanopen::Node;
protected:
    NodeId node_id_;
    ucan::Module& can_module_;

    ODEntry* dictionary_;
    size_t dictionary_size_;

    NmtState nmt_state_;
public:
    Server(ucan::Module& can_module,
           NodeId node_id,
           ODEntry* object_dictionary,
           size_t object_dictionary_size);

    NodeId node_id() const { return node_id_; }
    NmtState nmt_state() const { return nmt_state_; }
protected:
    virtual void on_sdo_overrun() {}
    virtual void on_rpdo_overrun() {}
private:
    void init_object_dictionary();
public:
    // const ODEntry* find_od_entry(ODObjectKey key) {
        // const ODEntry*
        // const ODEntry* entry = emb::binary_find(dictionary_,
        //                                         dictionary_ + dictionary_size_,
        //                                         key);
        // if (entry == dictionary_)
    // }
};

class FrameReceiver {
public:
    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const = 0;
    virtual void recv(const ucan::RxMessageAttribute&, const can_frame&) = 0;
    virtual void handle() = 0;
};

} // namespace impl
} // namespace ucanopen

#endif
#endif
