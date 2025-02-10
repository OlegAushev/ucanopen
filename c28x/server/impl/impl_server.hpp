#pragma once

#ifdef MCUDRV_C28X

#include <mcudrv/c28x/f2837xd/can/can.hpp>
#include <ucanopen/c28x/ucanopen_def.hpp>
#include <emblib/static_vector.hpp>
#include <algorithm>
#include <vector>

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
    std::vector<ODView> dicts_;
    NmtState nmt_state_;
private:
    emb::static_vector<mcu::c28x::can::MessageObject, 32> message_objects_;
public:
    Server(mcu::c28x::can::Module& can_module,
           NodeId node_id,
           const std::vector<ODView>& object_dictionaries);
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
        for (size_t i = 0; i < dicts_.size(); ++i) {
            std::pair<ODEntry*, ODEntry*> res = std::equal_range(
                    dicts_[i].begin, dicts_[i].begin + dicts_[i].size, key);
            if (res.first != res.second) {
                return res.first;
            }
        }
        return NULL;
    }
};

} // namesppace impl
} // namespace ucanopen

#endif
