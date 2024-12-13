#pragma once

#ifdef MCUDRV_C28X

#include <mcudrv/c28x/f2837xd/can/can.h>
#include <ucanopen/c28x/ucanopen_def.h>
#include <algorithm>

namespace ucanopen {

class HeartbeatService;
class TpdoService;
class RpdoService;
class SdoService;

namespace impl {

class Server {
    friend class ucanopen::HeartbeatService;
    friend class ucanopen::TpdoService;
    friend class ucanopen::RpdoService;
    friend class ucanopen::SdoService;
protected:
    NodeId node_id_;
    mcu::c28x::can::Module& can_module_;
    ODEntry* dictionary_;
    size_t dictionary_size_;
    NmtState nmt_state_;
private:
    emb::array<mcu::c28x::can::MessageObject, cob_count> message_objects_;
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
};

} // namesppace impl
} // namespace ucanopen

#endif
