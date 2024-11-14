#pragma once


#ifdef MCUDRV_C28X


#include "../ucanopen_def.h"
#include "impl/impl_server.h"
#include "services/heartbeat_service.h"
#include "services/rpdo_service.h"
#include "services/sdo_service.h"
#include "services/tpdo_service.h"
#include <mcudrv/c28x/f2837xd/chrono/chrono.h>


namespace ucanopen {


namespace impl {


extern unsigned char cana_rsdo_dualcore_alloc[sizeof(can_payload)];
extern unsigned char canb_rsdo_dualcore_alloc[sizeof(can_payload)];

extern unsigned char cana_tsdo_dualcore_alloc[sizeof(can_payload)];
extern unsigned char canb_tsdo_dualcore_alloc[sizeof(can_payload)];


} // namespace impl


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


class Server : public impl::Server, public emb::interrupt_invoker_array<Server, mcu::c28x::can::peripheral_count> {
protected:
    HeartbeatService* heartbeat_service;
    TpdoService* tpdo_service;
    RpdoService* rpdo_service;
    SdoService* sdo_service;

    virtual void on_run() {}
    virtual void on_can_bus_error() {}
public:
    Server(mcu::c28x::ipc::traits::singlecore, mcu::c28x::ipc::traits::primary, const IpcFlags& ipc_flags,
           mcu::c28x::can::Module* can_module, const ServerConfig& config,
           ODEntry* object_dictionary, size_t object_dictionary_size);

    Server(mcu::c28x::ipc::traits::dualcore, mcu::c28x::ipc::traits::primary, const IpcFlags& ipc_flags,
           mcu::c28x::can::Module* can_module, const ServerConfig& config);

    Server(mcu::c28x::ipc::traits::dualcore, mcu::c28x::ipc::traits::secondary, const IpcFlags& ipc_flags,
           mcu::c28x::can::Peripheral can_peripheral, ODEntry* object_dictionary, size_t object_dictionary_size);

    virtual ~Server() {}

    void enable() {
        if (this->_can_module) {
            this->_can_module->enable_interrupts();
            this->_nmt_state = NmtState::operational;
        }
    }

    void disable() {
        if (this->_can_module) {
            this->_can_module->disable_interrupts();
            this->_nmt_state = NmtState::stopped;
        }
    }

    void run();
private:
    static void on_frame_received(mcu::c28x::can::Module* can_module, uint32_t interrupt_cause, uint16_t status);
};


} // namespace ucanopen


#endif
