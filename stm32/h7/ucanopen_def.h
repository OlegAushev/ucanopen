#pragma once


#ifdef MCUDRV_STM32
#ifdef STM32H7xx


#include <cstdint>
#include <cstddef>
#include <cstring>
#include <array>
#include <optional>
#include <utility>
#include <emblib/interfaces/can.h>


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


enum class Cob : unsigned int {
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


constexpr size_t cob_count = 15;


constexpr std::array<can_id, cob_count> cob_function_codes = {
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


inline can_id calculate_cob_id(Cob cob, NodeId node_id) {
    if ((cob == Cob::nmt) || (cob == Cob::sync) || (cob == Cob::time)) {
        return cob_function_codes[std::to_underlying(cob)];
    }
    return cob_function_codes[std::to_underlying(cob)] + node_id.get();
}


constexpr std::array<size_t, cob_count> cob_sizes = {
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


enum class CobTpdo : unsigned int {
    tpdo1,
    tpdo2,
    tpdo3,
    tpdo4,
};


inline Cob to_cob_type(CobTpdo tpdo) {
    return static_cast<Cob>(std::to_underlying(Cob::tpdo1) + 2 * std::to_underlying(tpdo));
}


enum class CobRpdo : unsigned int {
    rpdo1,
    rpdo2,
    rpdo3,
    rpdo4,	
};


inline Cob to_cob_type(CobRpdo rpdo) {
    return static_cast<Cob>(std::to_underlying(Cob::rpdo1) + 2 * std::to_underlying(rpdo));
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
    bool bl;
    int16_t i16;
    int32_t i32;
    uint16_t u16;
    uint32_t u32;
    float f32;

    ExpeditedSdoData() : u32(0) {}
    ExpeditedSdoData(bool value) : u32(0) { bl = value; }
    ExpeditedSdoData(int16_t value) : u32(0) { i16 = value; }
    ExpeditedSdoData(int32_t value) : i32(value) {}
    ExpeditedSdoData(uint16_t value) : u32(0) { u16 = value; }
    ExpeditedSdoData(uint32_t value) : u32(value) {}
    ExpeditedSdoData(float value) : f32(value) {}
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
    ExpeditedSdo() : data_size_indicated(0), expedited_transfer(0), data_empty_bytes(0), _reserved(0)
                , cs(0), index(0), subindex(0), data() {}
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


enum ODObjectDataType {
    OD_BOOL,
    OD_INT8,
    OD_INT16,
    OD_INT32,
    OD_UINT8,
    OD_UINT16,
    OD_UINT32,
    OD_FLOAT32,
    OD_EXEC,
    OD_STRING
};


enum ODObjectAccessPermission {
    OD_ACCESS_RW,
    OD_ACCESS_RO,
    OD_ACCESS_WO,
    OD_ACCESS_CONST
};


// Used in OD-entries for default values definition
#define OD_NO_DEFAULT_VALUE std::nullopt
#define OD_DEFAULT_VALUE(value) ExpeditedSdoData(value)


// Used in OD-entries which doesn't have direct access to data through pointer.
#define OD_NO_DIRECT_ACCESS std::pair<uint32_t*, uint32_t**>(nullptr, nullptr)


// Used in OD-entries which have direct access to data through pointer.
#define OD_PTR(ptr) std::pair<uint32_t*, uint32_t**>(reinterpret_cast<uint32_t*>(ptr), nullptr)
#define OD_DPTR(dptr) std::pair<uint32_t*, uint32_t**>(nullptr, reinterpret_cast<uint32_t**>(dptr))


// Used in OD-entries which don't have read access to data through function.
inline SdoAbortCode OD_NO_INDIRECT_READ_ACCESS(ExpeditedSdoData& retval) { return SdoAbortCode::unsupported_access; }


// Used in OD-entries which don't have write access to data through function.
inline SdoAbortCode OD_NO_INDIRECT_WRITE_ACCESS(ExpeditedSdoData val) { return SdoAbortCode::unsupported_access; }


const size_t od_object_type_sizes[10] = {sizeof(bool), sizeof(int8_t), sizeof(int16_t), sizeof(int32_t),
                                        sizeof(uint8_t), sizeof(uint16_t), sizeof(uint32_t), sizeof(float),
                                        4, 4};


struct ODObjectKey {
    uint16_t index;
    uint16_t subindex;
};


struct ODObject {
    const char* category;
    const char* subcategory;
    const char* name;
    const char* unit;
    ODObjectAccessPermission access_permission;
    ODObjectDataType data_type;
    std::optional<ExpeditedSdoData> default_value;
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


inline bool operator==(const ODObjectKey& lhs, const ODObjectKey& rhs) {
    return (lhs.index == rhs.index) && (lhs.subindex == rhs.subindex);
}


enum class Error {
    none                            = uint32_t(0),
    can_bus_error                   = uint32_t(1) << 0,
    can_bus_overrun                 = uint32_t(1) << 1,
    can_bus_connection_lost         = uint32_t(1) << 2,
    can_bus_checksum_mismatch       = uint32_t(1) << 3,
    can_bus_counter_freeze          = uint32_t(1) << 4,
};


enum class Warning {
    none                            = uint32_t(0),
    can_bus_error                   = uint32_t(1) << 0,
    can_bus_overrun                 = uint32_t(1) << 1,
    can_bus_connection_lost         = uint32_t(1) << 2,
    can_bus_checksum_mismatch       = uint32_t(1) << 3,
    can_bus_counter_freeze          = uint32_t(1) << 4,
};


} // namespace ucanopen


#endif
#endif
