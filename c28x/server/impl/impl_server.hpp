#pragma once

#ifdef MCUDRV_C28X

#include <mcudrv/c28x/f2837xd/can/can.hpp>
#include <ucanopen/c28x/ucanopen_def.hpp>
#include <algorithm>
#include <emblib/static_vector.hpp>

namespace ucanopen {

class HeartbeatService;
class TpdoService;
class RpdoService;
class SdoService;
class Node;

namespace impl {

class Server {
    friend class ucanopen::HeartbeatService;
    friend class ucanopen::TpdoService;
    friend class ucanopen::RpdoService;
    friend class ucanopen::SdoService;
    friend class ucanopen::Node;
protected:
    NodeId node_id_;
    mcu::c28x::can::Module& can_module_;
    ODEntry* dictionary_;
    size_t dictionary_size_;
    NmtState nmt_state_;
private:
    emb::static_vector<mcu::c28x::can::MessageObject, 32> message_objects_;
public:
    Server(mcu::c28x::can::Module& can_module,
           NodeId node_id,
           ODEntry* object_dictionary,
           size_t object_dictionary_size);
    virtual ~Server() {}
    NodeId node_id() const { return node_id_; }
    NmtState nmt_state() const { return nmt_state_; }
protected:
    virtual void on_sdo_overrun() {}
    virtual void on_rpdo_overrun() {}
private:
    void init_message_objects();
    void init_object_dictionary();
public:
    const ODEntry* find_od_entry(ODObjectKey key) {
        std::pair<ODEntry*, ODEntry*> res = std::equal_range(
                dictionary_, dictionary_ + dictionary_size_, key);
        if (res.first == res.second) {
            return NULL;
        }
        return res.first;
    }
};

} // namesppace impl
} // namespace ucanopen

#endif
