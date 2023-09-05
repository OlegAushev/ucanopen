#pragma once


#ifdef STM32F4xx


#include <cstdint>
#include <cstddef>
#include <cstring>
#include <array>
#include <utility>
#include <emblib_stm32/interfaces/can.h>


namespace ucanopen {

template <typename T>
inline can_payload to_payload(const T& message) {
    static_assert(sizeof(T) <= 8);
    can_payload payload = {};
    memcpy(payload.data(), &message, sizeof(T));
    return payload;
}


template <typename T>
inline T from_payload(const can_payload& payload) {
    static_assert(sizeof(T) <= 8);
    T message = {};
    memcpy(&message, payload.data(), sizeof(T));
    return message;
}


class NodeId {
private:
    unsigned int _id;
public:
    explicit NodeId(unsigned int id) : _id(id) {}
    unsigned int get() const { return _id; }
    bool valid() const { return (_id >= 1) && (_id <= 127); }
};


enum class NmtState : uint8_t {
    initializing = 0x00,
    stopped = 0x04,
    operational = 0x05,
    pre_operational = 0x7F
};


enum class CobType {
    nmt,
    sync,
    emcy,
    time,
    tpdo1,
    rpdo1,
    tpdo2,
    rpdo2,
    tpdo3,
    rpdo3,
    tpdo4,
    rpdo4,
    tsdo,
    rsdo,
    heartbeat
};


constexpr int cob_type_count = 15;


constexpr std::array<can_id, cob_type_count> cob_function_codes = {
    0x000,  // NMT
    0x080,  // SYNC
    0x080,  // EMCY
    0x100,  // TIME
    0x180,  // TPDO1
    0x200,  // RPDO1
    0x280,  // TPDO2
    0x300,  // RPDO2
    0x380,  // TPDO3
    0x400,  // RPDO3
    0x480,  // TPDO4
    0x500,  // RPDO4
    0x580,  // TSDO
    0x600,  // RSDO
    0x700   // HEARTBEAT
};


inline can_id calculate_cob_id(CobType cob_type, NodeId node_id) {
    if ((cob_type == CobType::nmt) || (cob_type == CobType::sync) || (cob_type == CobType::time)) {
        return cob_function_codes[std::to_underlying(cob_type)];
    }
    return cob_function_codes[std::to_underlying(cob_type)] + node_id.get();
}


constexpr std::array<int, cob_type_count> cob_sizes = {
    2,  // NMT
    0,  // SYNC
    2,  // EMCY
    6,  // TIME
    8,  // TPDO1
    8,  // RPDO1
    8,  // TPDO2
    8,  // RPDO2
    8,  // TPDO3
    8,  // RPDO3
    8,  // TPDO4
    8,  // RPDO4
    8,  // TSDO
    8,  // RSDO
    1   // HEARTBEAT
};


enum class TpdoType {
    tpdo1,
    tpdo2,
    tpdo3,
    tpdo4,
};


inline CobType to_cob_type(TpdoType tpdo_type) {
    return static_cast<CobType>(std::to_underlying(CobType::tpdo1) + 2 * std::to_underlying(tpdo_type));
}


enum class RpdoType {
    rpdo1,
    rpdo2,
    rpdo3,
    rpdo4,	
};


inline CobType to_cob_type(RpdoType rpdo_type) {
    return static_cast<CobType>(std::to_underlying(CobType::rpdo1) + 2 * std::to_underlying(rpdo_type));
}


namespace sdo_cs_codes {
const uint32_t client_init_write = 1;
const uint32_t server_init_write = 3;
const uint32_t client_init_read = 2;
const uint32_t server_init_read = 2;

const uint32_t abort = 4;
}


inline uint32_t get_cs_code(const can_frame& frame) {
    return frame.payload[0] >> 5;
}


union ExpeditedSdoData {
    int32_t i32;
    uint32_t u32;
    float f32;
};


struct ExpeditedSdo {
    uint32_t data_size_indicated : 1;
    uint32_t expedited_transfer : 1;
    uint32_t data_empty_bytes : 2;
    uint32_t _reserved : 1;
    uint32_t cs : 3;
    uint32_t index : 16;
    uint32_t subindex : 8;
    ExpeditedSdoData data;
    ExpeditedSdo() { memset(this, 0, sizeof(ExpeditedSdo)); }
};


struct AbortSdo {
    uint32_t _reserved : 5;
    uint32_t cs : 3;
    uint32_t index : 16;
    uint32_t subindex : 8;
    uint32_t error_code;
    AbortSdo() {
        memset(this, 0, sizeof(AbortSdo));
        cs = sdo_cs_codes::abort;
    }
};


enum class SdoAbortCode : uint32_t {
    no_error                = 0,
    invalid_cs              = 0x05040001,
    unsupported_access      = 0x06010000,
    read_access_wo          = 0x06010001,
    write_access_ro         = 0x06010002,
    object_not_found        = 0x06020000,
    hardware_error          = 0x06060000,
    value_range_exceeded    = 0x06090030,
    value_too_high          = 0x06090031,
    value_too_low           = 0x06090032,
    general_error           = 0x08000000,
    data_store_error        = 0x08000020,
    local_control_error     = 0x08000021,
    state_error             = 0x08000022
};


enum ODObjectType {
    OD_BOOL,
    OD_INT16,
    OD_INT32,
    OD_UINT16,
    OD_UINT32,
    OD_FLOAT32,
    OD_ENUM16,
    OD_EXEC,
    OD_STRING
};


enum ODObjectAccessPermission {
    OD_ACCESS_RW,
    OD_ACCESS_RO,
    OD_ACCESS_WO,
    OD_ACCESS_CONST
};


// Used in OD-entries which doesn't have direct access to data through pointer.
#define OD_NO_DIRECT_ACCESS std::pair<uint32_t*, uint32_t**>(nullptr, nullptr)


// Used in OD-entries which have direct access to data through pointer.
#define OD_PTR(ptr) reinterpret_cast<uint32_t*>(ptr)
#define OD_DPTR(dptr) reinterpret_cast<uint32_t**>(dptr)


// Used in OD-entries which don't have read access to data through function.
inline SdoAbortCode OD_NO_INDIRECT_READ_ACCESS(ExpeditedSdoData& retval) { return SdoAbortCode::unsupported_access; }


// Used in OD-entries which don't have write access to data through function.
inline SdoAbortCode OD_NO_INDIRECT_WRITE_ACCESS(ExpeditedSdoData val) { return SdoAbortCode::unsupported_access; }


const int od_object_type_sizes[9] = {sizeof(bool), sizeof(int16_t), sizeof(int32_t),
                                     sizeof(uint16_t), sizeof(uint32_t), sizeof(float),
                                     sizeof(uint16_t), 0, 0};


struct ODObjectKey {
    uint32_t index;
    uint32_t subindex;
};


struct ODObject {
    const char* category;
    const char* subcategory;
    const char* name;
    const char* unit;
    ODObjectType type;
    ODObjectAccessPermission access_permission;
    std::pair<uint32_t*, uint32_t**> ptr;
    SdoAbortCode (*read_func)(ExpeditedSdoData& retval);
    SdoAbortCode (*write_func)(ExpeditedSdoData val);

    bool has_direct_access() const {
        return ptr != OD_NO_DIRECT_ACCESS;
    }

    bool has_read_permission() const {
        return access_permission != OD_ACCESS_WO;
    }

    bool has_write_permission() const {
        return (access_permission == OD_ACCESS_RW) || (access_permission == OD_ACCESS_WO);
    }
};


struct ODEntry {
    ODObjectKey key;
    ODObject object;
};


inline bool operator<(const ODEntry& lhs, const ODEntry& rhs) {
    return (lhs.key.index < rhs.key.index)
        || ((lhs.key.index == rhs.key.index) && (lhs.key.subindex < rhs.key.subindex));
}


inline bool operator<(const ODObjectKey& lhs, const ODEntry& rhs) {
    return (lhs.index < rhs.key.index)
        || ((lhs.index == rhs.key.index) && (lhs.subindex < rhs.key.subindex));
}


inline bool operator==(const ODObjectKey& lhs, const ODEntry& rhs) {
    return (lhs.index == rhs.key.index) && (lhs.subindex == rhs.key.subindex);
}

} // namespace ucanopen

#endif
