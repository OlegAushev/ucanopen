#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32H7xx


#include "../ucanopen_def.h"
#include "impl/impl_server.h"
#include "services/heartbeat_service.h"
#include "services/rpdo_service.h"
#include "services/sdo_service.h"
#include "services/sync_service.h"
#include "services/tpdo_service.h"
#include "../node/node.h"

#include <bitset>
#include <vector>


namespace ucanopen {


struct ServerConfig {
    uint32_t node_id;
    uint32_t heartbeat_period_ms;   // 0 - is inactive
    uint32_t sync_period_ms;        // 0 - is inactive
    uint32_t tpdo1_period_ms;       // 0 - TPDO is inactive
    uint32_t tpdo2_period_ms;
    uint32_t tpdo3_period_ms;
    uint32_t tpdo4_period_ms;
    uint32_t rpdo1_timeout_ms;      // 0 - no RPDO timeout
    uint32_t rpdo2_timeout_ms;
    uint32_t rpdo3_timeout_ms;
    uint32_t rpdo4_timeout_ms;
    uint32_t rpdo1_id;              // 0 - use default RPDO ID
    uint32_t rpdo2_id;
    uint32_t rpdo3_id;
    uint32_t rpdo4_id;
};


class Server : public impl::Server, public emb::interrupt_invoker_array<Server, mcu::can::peripheral_count> {
protected:
    HeartbeatService* heartbeat_service;
    SyncService* sync_service;
    TpdoService* tpdo_service;
    RpdoService* rpdo_service;
    SdoService* sdo_service;

    std::vector<Node*> nodes;
    std::vector<std::pair<mcu::can::MessageAttribute, impl::FrameReceiverInterface*>> _attr_map;
    virtual void on_run() {}

    uint64_t _errcount = 0;
    std::bitset<32> _connection_status = 0;
public:
    Server(mcu::can::Module& can_module, const ServerConfig& config,
           ODEntry* object_dictionary, size_t object_dictionary_size);

    void add_node(Node* node_);
    void start();
    void stop();
    void run();

    static void on_frame_received(mcu::can::Module& can_module, const mcu::can::MessageAttribute& attr, const can_frame& frame);
};


} // namespace ucanopen


#endif
#endif
