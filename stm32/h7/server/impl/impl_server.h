#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32H7xx


#include "../../ucanopen_def.h"
#include <mcudrv/stm32/h7/can/can.h>
#include <algorithm>
#include <vector>

namespace ucan = mcu::stm32::can;

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
    NodeId _node_id;
    ucan::Module& _can_module;

    ODEntry* _dictionary;
    size_t _dictionary_size;

    NmtState _nmt_state;
public:
    Server(ucan::Module& can_module, NodeId node_id,
           ODEntry* object_dictionary, size_t object_dictionary_size);

    NodeId node_id() const { return _node_id; }
    NmtState nmt_state() const { return _nmt_state; }
private:
    void _init_object_dictionary();
};


} // namespace impl


enum class FrameRecvStatus {
    success,
    attr_mismatch,
    overrun,
    //invalid_format,
    //object_not_found,
    //irrelevant_frame
};


namespace impl {


class FrameReceiver {
public:
    virtual std::vector<ucan::RxMessageAttribute> get_rx_attr() const = 0;
    virtual FrameRecvStatus recv_frame(const ucan::RxMessageAttribute&, const can_frame&) = 0;
    virtual void handle_recv_frames() = 0;
};


} // namespace impl


} // namespace ucanopen


#endif
#endif
