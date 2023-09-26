#pragma once


#ifdef STM32F4xx


#include "../server/server.h"


namespace ucanopen {

namespace tests {

struct CobTpdo1 {
    int64_t clock;
    CobTpdo1() {
        static_assert(sizeof(CobTpdo1) == 8);
        memset(this, 0, sizeof(CobTpdo1));
    }
};


struct CobTpdo2 {
    uint32_t milliseconds;
    uint32_t seconds;
    CobTpdo2() {
        static_assert(sizeof(CobTpdo2) == 8);
        memset(this, 0, sizeof(CobTpdo2));
    }
};


struct CobTpdo3 {
    uint16_t value_from_rpdo1 : 16;
    uint16_t value_from_rpdo2 : 16;
    uint16_t value_from_rpdo3 : 16;
    uint16_t value_from_rpdo4 : 16;
    CobTpdo3() {
        static_assert(sizeof(CobTpdo3) == 8);
        memset(this, 0, sizeof(CobTpdo3));
    }
};


struct CobTpdo4 {
    uint64_t counter : 2;
    uint64_t errors : 31;
    uint64_t warnings : 31;
    CobTpdo4() {
        static_assert(sizeof(CobTpdo4) == 8);
        memset(this, 0, sizeof(CobTpdo4));
    }
};


//----------------------------------------------------------------------------------------------------------------------
struct CobRpdo1 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
    CobRpdo1() {
        static_assert(sizeof(CobRpdo1) == 8);
        memset(this, 0, sizeof(CobRpdo1));
    }
};


struct CobRpdo2 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
    CobRpdo2() {
        static_assert(sizeof(CobRpdo2) == 8);
        memset(this, 0, sizeof(CobRpdo2));
    }
};


struct CobRpdo3 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
    CobRpdo3() {
        static_assert(sizeof(CobRpdo3) == 8);
        memset(this, 0, sizeof(CobRpdo3));
    }
};


struct CobRpdo4 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
    CobRpdo4() {
        static_assert(sizeof(CobRpdo4) == 8);
        memset(this, 0, sizeof(CobRpdo4));
    }
};
//----------------------------------------------------------------------------------------------------------------------
extern ODEntry object_dictionary[];
extern const int object_dictionary_size;


const inline ucanopen::ServerConfig config1 = {
    .node_id = 0x01,
    .heartbeat_period_ms = 1000,
    .sync_period_ms = 100,
    .tpdo1_period_ms = 50,
    .tpdo2_period_ms = 75,
    .tpdo3_period_ms = 100,
    .tpdo4_period_ms = 125,
    .rpdo1_timeout_ms = 1000,
    .rpdo2_timeout_ms = 1000,
    .rpdo3_timeout_ms = 1000,
    .rpdo4_timeout_ms = 1000,
    .rpdo1_id = 0x182,
    .rpdo2_id = 0x282,
    .rpdo3_id = 0,
    .rpdo4_id = 0,
};


const inline ucanopen::ServerConfig config2 = {
    .node_id = 0x02,
    .heartbeat_period_ms = 1000,
    .sync_period_ms = 100,
    .tpdo1_period_ms = 50,
    .tpdo2_period_ms = 75,
    .tpdo3_period_ms = 100,
    .tpdo4_period_ms = 125,
    .rpdo1_timeout_ms = 1000,
    .rpdo2_timeout_ms = 1000,
    .rpdo3_timeout_ms = 1000,
    .rpdo4_timeout_ms = 1000,
    .rpdo1_id = 0x182,
    .rpdo2_id = 0x282,
    .rpdo3_id = 0,
    .rpdo4_id = 0,
};


class Server : public ucanopen::Server {
public:
    Server(mcu::can::Module& can_module, const ServerConfig& config)
            : ucanopen::Server(can_module, config, object_dictionary, object_dictionary_size) {
        tpdo_service->register_tpdo(TpdoType::tpdo1, std::chrono::milliseconds(config.tpdo1_period_ms), _create_tpdo1);
        tpdo_service->register_tpdo(TpdoType::tpdo2, std::chrono::milliseconds(config.tpdo2_period_ms), _create_tpdo2);
        tpdo_service->register_tpdo(TpdoType::tpdo3, std::chrono::milliseconds(config.tpdo3_period_ms), _create_tpdo3);
        tpdo_service->register_tpdo(TpdoType::tpdo4, std::chrono::milliseconds(config.tpdo4_period_ms), _create_tpdo4);

        rpdo_service->register_rpdo(RpdoType::rpdo1, std::chrono::milliseconds(config.rpdo1_timeout_ms), _handle_rpdo1, config.rpdo1_id);
        rpdo_service->register_rpdo(RpdoType::rpdo2, std::chrono::milliseconds(config.rpdo2_timeout_ms), _handle_rpdo2, config.rpdo2_id);
        rpdo_service->register_rpdo(RpdoType::rpdo3, std::chrono::milliseconds(config.rpdo3_timeout_ms), _handle_rpdo3, config.rpdo3_id);
        rpdo_service->register_rpdo(RpdoType::rpdo4, std::chrono::milliseconds(config.rpdo4_timeout_ms), _handle_rpdo4, config.rpdo4_id);
    }

    virtual void on_run() override {
        static bool warning_detected = false;
        static auto warning_timepoint = std::chrono::milliseconds(0);

        if (syslog::has_warning(sys::Warning::can_bus_connection_lost)) {
            if (!warning_detected) {
                warning_detected = true;
                warning_timepoint = mcu::chrono::system_clock::now();
            }

            if (mcu::chrono::system_clock::now() > warning_timepoint + std::chrono::milliseconds(5000)) {
                syslog::set_error(sys::Error::can_bus_connection_lost);
            }
        } else {
            warning_detected = false;
        }
    }

private:
    static inline float value_from_rpdo1 = 0;
    static inline float value_from_rpdo2 = 0;
    static inline float value_from_rpdo3 = 0;
    static inline float value_from_rpdo4 = 0;

    static can_payload _create_tpdo1() {
        CobTpdo1 tpdo;
        tpdo.clock = mcu::chrono::system_clock::now().count();
        return to_payload<CobTpdo1>(tpdo);
    }

    static can_payload _create_tpdo2() {
        CobTpdo2 tpdo;
        tpdo.seconds = static_cast<uint32_t>(mcu::chrono::system_clock::now().count() / 1000);
        tpdo.milliseconds = static_cast<uint32_t>(mcu::chrono::system_clock::now().count() - 1000 * tpdo.seconds);
        return to_payload<CobTpdo2>(tpdo);
    }

    static can_payload _create_tpdo3() {
        CobTpdo3 tpdo;
        tpdo.value_from_rpdo1 = static_cast<uint16_t>(value_from_rpdo1);
        tpdo.value_from_rpdo2 = static_cast<uint16_t>(value_from_rpdo2);
        tpdo.value_from_rpdo3 = static_cast<uint16_t>(value_from_rpdo3);
        tpdo.value_from_rpdo4 = static_cast<uint16_t>(value_from_rpdo4);
        return to_payload<CobTpdo3>(tpdo);
    }

    static can_payload _create_tpdo4() {
        static unsigned int counter = 0;
        CobTpdo4 tpdo;
        tpdo.counter = counter & 0x3;
        //tpdo.errors = syslog::errors();
        //tpdo.warnings = syslog::warnings();
        counter = (counter + 1) % 4;
        return to_payload<CobTpdo4>(tpdo);
    }

    static void _handle_rpdo1(const can_payload& payload) {
        CobRpdo1 rpdo = from_payload<CobRpdo1>(payload);
        value_from_rpdo1 = rpdo.value;
    }

    static void _handle_rpdo2(const can_payload& payload) {
        CobRpdo2 rpdo = from_payload<CobRpdo2>(payload);
        value_from_rpdo2 = rpdo.value;
    }

    static void _handle_rpdo3(const can_payload& payload) {
        CobRpdo3 rpdo = from_payload<CobRpdo3>(payload);
        value_from_rpdo3 = rpdo.value;
    }

    static void _handle_rpdo4(const can_payload& payload) {
        CobRpdo4 rpdo = from_payload<CobRpdo4>(payload);
        value_from_rpdo4 = rpdo.value;
    }
};

} // namespace tests

} // namespae ucanopen

#endif
