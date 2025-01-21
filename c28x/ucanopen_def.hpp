#pragma once

#ifdef MCUDRV_C28X

#include <mcudrv/generic/can.hpp>
#include <emblib/core.hpp>
#include <emblib/optional.hpp>
#include <cstring>
#include <utility>

namespace ucanopen {

template <typename T>
inline can_payload to_payload(const T& message) {
    EMB_STATIC_ASSERT(sizeof(T) <= 4);
    can_payload payload;
    payload.fill(0);
    emb::c28x::to_bytes(payload.data, message);
    return payload;
}

template <typename T>
inline void to_payload(can_payload& payload, const T& message) {
    EMB_STATIC_ASSERT(sizeof(T) <= 4);
    payload.fill(0);
    emb::c28x::to_bytes(payload.data, message);
}

template <typename T>
inline T from_payload(const can_payload& payload) {
    EMB_STATIC_ASSERT(sizeof(T) <= 4);
    T message;
    emb::c28x::from_bytes(message, payload.data);
    return message;
}

class NodeId {
private:
    unsigned int id_;
public:
    explicit NodeId(unsigned int id) : id_(id) {}
    unsigned int get() const { return id_; }
    bool valid() const { return (id_ >= 1) && (id_ <= 127); }
};

SCOPED_ENUM_UT_DECLARE_BEGIN(NmtState, uint32_t) {
    initializing = 0x00,
    stopped = 0x04,
    operational = 0x05,
    pre_operational = 0x7F
} SCOPED_ENUM_DECLARE_END(NmtState)

const size_t cob_count = 16;
SCOPED_ENUM_UT_DECLARE_BEGIN(Cob, uint32_t) {
    dummy,
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
} SCOPED_ENUM_DECLARE_END(Cob)

const emb::array<uint32_t, cob_count> cob_function_codes = {
    0x000,  // DUMMY
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

inline uint32_t calculate_cob_id(Cob cob, NodeId node_id) {
    if ((cob == Cob::nmt) || (cob == Cob::sync) || (cob == Cob::time)) {
        return cob_function_codes[cob.underlying_value()];
    }
    return cob_function_codes[cob.underlying_value()] + node_id.get();
}

const emb::array<int, cob_count> cob_data_len = {
    0,  // DUMMY
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

SCOPED_ENUM_UT_DECLARE_BEGIN(CobTpdo, uint32_t) {
    tpdo1,
    tpdo2,
    tpdo3,
    tpdo4,
} SCOPED_ENUM_DECLARE_END(CobTpdo)

inline Cob to_cob(CobTpdo tpdo) {
    return Cob(static_cast<uint32_t>(Cob::tpdo1) + 2 * tpdo.underlying_value());
}

SCOPED_ENUM_UT_DECLARE_BEGIN(CobRpdo, uint32_t) {
    rpdo1,
    rpdo2,
    rpdo3,
    rpdo4,
} SCOPED_ENUM_DECLARE_END(CobRpdo)

inline Cob to_cob(CobRpdo rpdo) {
    return Cob(static_cast<uint32_t>(Cob::rpdo1) + 2 * rpdo.underlying_value());
}

namespace sdo_cs_codes {
const uint32_t client_init_write = 1;
const uint32_t server_init_write = 3;
const uint32_t client_init_read = 2;
const uint32_t server_init_read = 2;

const uint32_t abort = 4;
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

SCOPED_ENUM_UT_DECLARE_BEGIN(SdoAbortCode, uint32_t) {
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
} SCOPED_ENUM_DECLARE_END(SdoAbortCode)

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
#define OD_NO_DEFAULT_VALUE emb::nullopt
#define OD_DEFAULT_VALUE(value) ExpeditedSdoData(value)

// Used in OD-entries which doesn't have direct access to data through pointer.
#define OD_NO_DIRECT_ACCESS std::pair<uint32_t*, uint32_t**>(NULL, NULL)

// Used in OD-entries which have direct access to data through pointer.
#define OD_PTR(ptr) \
    std::pair<uint32_t*, uint32_t**>(reinterpret_cast<uint32_t*>(ptr), NULL)
#define OD_DPTR(dptr) \
    std::pair<uint32_t*, uint32_t**>(NULL, reinterpret_cast<uint32_t**>(dptr))

// Used in OD-entries which don't have read access to data through function.
inline SdoAbortCode OD_NO_INDIRECT_READ_ACCESS(ExpeditedSdoData& retval) {
    return SdoAbortCode::unsupported_access;
}

// Used in OD-entries which don't have write access to data through function.
inline SdoAbortCode OD_NO_INDIRECT_WRITE_ACCESS(ExpeditedSdoData val) {
    return SdoAbortCode::unsupported_access;
}

const size_t od_object_type_sizes[10] = {sizeof(bool),
                                         sizeof(int8_t),
                                         sizeof(int16_t),
                                         sizeof(int32_t),
                                         sizeof(uint8_t),
                                         sizeof(uint16_t),
                                         sizeof(uint32_t),
                                         sizeof(float),
                                         2,
                                         2};

struct ODObjectKey {
    uint16_t index;
    uint8_t subindex;
};

struct ODObject {
    const char* category;
    const char* subcategory;
    const char* name;
    const char* unit;
    ODObjectAccessPermission access_permission;
    ODObjectDataType data_type;
    emb::optional<ExpeditedSdoData> default_value;
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
        return (access_permission == OD_ACCESS_RW) ||
               (access_permission == OD_ACCESS_WO);
    }
};

struct ODEntry {
    ODObjectKey key;
    ODObject object;
};

inline bool operator<(const ODEntry& lhs, const ODEntry& rhs) {
    return (lhs.key.index < rhs.key.index) ||
           ((lhs.key.index == rhs.key.index) &&
            (lhs.key.subindex < rhs.key.subindex));
}

inline bool operator<(const ODEntry& lhs, const ODObjectKey& rhs) {
    return (lhs.key.index < rhs.index) ||
           ((lhs.key.index == rhs.index) && (lhs.key.subindex < rhs.subindex));
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


} // namespace ucanopen


#endif
