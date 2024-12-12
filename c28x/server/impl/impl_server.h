#pragma once


#ifdef MCUDRV_C28X


#include <mcudrv/c28x/f2837xd/can/can.h>
#include <mcudrv/c28x/f2837xd/ipc/ipc.h>
#include <ucanopen/c28x/ucanopen_def.h>
#include <algorithm>


namespace ucanopen {


struct IpcFlags {
    mcu::c28x::ipc::NewFlag rpdo1_received;
    mcu::c28x::ipc::NewFlag rpdo2_received;
    mcu::c28x::ipc::NewFlag rpdo3_received;
    mcu::c28x::ipc::NewFlag rpdo4_received;
    mcu::c28x::ipc::NewFlag rsdo_received;
    mcu::c28x::ipc::NewFlag tsdo_ready;
};


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
    const mcu::c28x::ipc::Mode _ipc_mode;
    const mcu::c28x::ipc::Role _ipc_role;

    NodeId _node_id;
    mcu::c28x::can::Peripheral _can_peripheral;
    mcu::c28x::can::Module* _can_module;

    ODEntry* _dictionary;
    size_t _dictionary_size;

    NmtState _nmt_state;
private:
    emb::array<mcu::c28x::can::MessageObject, cob_count> _message_objects;
public:
    Server(mcu::c28x::ipc::traits::singlecore, mcu::c28x::ipc::traits::primary,
           mcu::c28x::can::Module* can_module, NodeId node_id,
           ODEntry* object_dictionary, int object_dictionary_size);

    Server(mcu::c28x::ipc::traits::dualcore, mcu::c28x::ipc::traits::primary,
           mcu::c28x::can::Module* can_module, NodeId node_id);

    Server(mcu::c28x::ipc::traits::dualcore, mcu::c28x::ipc::traits::secondary,
           mcu::c28x::can::Peripheral can_peripheral, ODEntry* object_dictionary, int object_dictionary_size);

    virtual ~Server() {}

    NodeId node_id() const { return _node_id; }
    NmtState nmt_state() const { return _nmt_state; }
protected:
    virtual void on_sdo_overrun() {}
    virtual void on_rpdo_overrun() {}
private:
    void _init_message_objects();
    void _init_object_dictionary();
};


} // namesppace impl


} // namespace ucanopen


#endif
