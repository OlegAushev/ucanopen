#pragma once

#ifdef MCUDRV_C28X

#include "../ucanopen_def.h"
#include "impl/impl_server.h"
#include "services/heartbeat_service.h"
#include "services/rpdo_service.h"
#include "services/sdo_service.h"
#include "services/tpdo_service.h"
#include "../node/node.h"

namespace ucanopen {

struct ServerConfig {
    uint32_t node_id;
    uint32_t heartbeat_period_ms;
    uint32_t tpdo1_period_ms;	// 0 - TPDO is inactive
    uint32_t tpdo2_period_ms;
    uint32_t tpdo3_period_ms;
    uint32_t tpdo4_period_ms;
    uint32_t rpdo1_timeout_ms;	// 0 - no RPDO timeout
    uint32_t rpdo2_timeout_ms;
    uint32_t rpdo3_timeout_ms;
    uint32_t rpdo4_timeout_ms;
    uint32_t rpdo1_id;			// 0 - use default RPDO ID
    uint32_t rpdo2_id;
    uint32_t rpdo3_id;
    uint32_t rpdo4_id;
};

class Server
        : public impl::Server,
          public emb::interrupt_invoker_array<Server, mcu::c28x::can::peripheral_count> {
protected:
    HeartbeatService* heartbeat_service;
    TpdoService* tpdo_service;
    RpdoService* rpdo_service;
    SdoService* sdo_service;

    virtual void on_run() {}
    virtual void on_can_bus_error() {}
public:
    Server(mcu::c28x::can::Module& can_module,
           const ServerConfig& config,
           ODEntry* object_dictionary,
           size_t object_dictionary_size);
    virtual ~Server() {}

    void enable() {
        this->can_module_.enable_interrupts();
        this->nmt_state_ = NmtState::operational;
    }

    void disable() {
        this->can_module_.disable_interrupts();
        this->nmt_state_ = NmtState::stopped;
    }

    void run();
private:
    static void on_frame_received(mcu::c28x::can::Module* can_module,
                                  uint32_t interrupt_cause,
                                  uint16_t status);
private:
    emb::array<Node*, 16> node_map_;
    emb::static_vector<Node*, 8> nodes_;
public:
    void add_node(Node* node) {
        nodes_.push_back(node);
    }

    void register_node_cob(Node* node, uint32_t obj_id) {
        if (obj_id >= node_map_.size()) {
            return;
        }
        node_map_[obj_id] = node;
    }
};

} // namespace ucanopen

#endif
