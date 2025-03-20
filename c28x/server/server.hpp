#pragma once

#ifdef MCUDRV_C28X

#include <ucanopen/c28x/ucanopen_def.hpp>
#include <ucanopen/c28x/server/impl/impl_server.hpp>
#include <ucanopen/c28x/server/services/heartbeat_service.hpp>
#include <ucanopen/c28x/server/services/rpdo_service.hpp>
#include <ucanopen/c28x/server/services/sdo_service.hpp>
#include <ucanopen/c28x/server/services/tpdo_service.hpp>
#include <ucanopen/c28x/node/node.hpp>

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
          public emb::singleton_array<Server, mcu::c28x::can::peripheral_count> {
protected:
    HeartbeatService* heartbeat_service;
    TpdoService* tpdo_service;
    RpdoService* rpdo_service;
    SdoService* sdo_service;

    virtual void inspect() {}
    virtual void on_can_bus_error() {}
public:
    Server(mcu::c28x::can::Module& can_module,
           const ServerConfig& config,
           const std::vector<ODView>& object_dictionaries);

    virtual ~Server() {}

    void start() {
        this->can_module_.enable_interrupts();
        this->nmt_state_ = NmtState::operational;
    }

    void stop() {
        this->can_module_.disable_interrupts();
        this->nmt_state_ = NmtState::stopped;
    }

    void run();
private:
    static void on_frame_received(mcu::c28x::can::Module* can_module,
                                  uint32_t interrupt_cause,
                                  uint16_t status);
private:
    emb::static_vector<Node*, 8> nodes_;
    emb::array<Node*, 16> node_map_;
public:
    void add_node(Node* node);
    void register_node_cob(Node* node, uint32_t obj_id);
};

} // namespace ucanopen

#endif
