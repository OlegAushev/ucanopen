#pragma once


#include <c28x_emb/emb_array.h>
#include <c28x_emb/emb_core.h>
#include <cstring>


namespace ucanopen {

typedef emb::Array<uint16_t, 8> can_payload;


template <typename T>
inline can_payload to_payload(const T& message)
{
	EMB_STATIC_ASSERT(sizeof(T) <= 4);
	can_payload payload;
	payload.fill(0);
	emb::c28x::to_bytes(payload.data, message);
	return payload;
}


template <typename T>
inline void to_payload(can_payload& payload, const T& message)
{
	EMB_STATIC_ASSERT(sizeof(T) <= 4);
	payload.fill(0);
	emb::c28x::to_bytes(payload.data, message);
}


template <typename T>
inline T from_payload(const can_payload& payload)
{
	EMB_STATIC_ASSERT(sizeof(T) <= 4);
	T message;
	emb::c28x::from_bytes(message, payload.data);
	return message;
}


class NodeId
{
private:
	unsigned int _value;
public:
	explicit NodeId(unsigned int value) : _value(value) {}
	unsigned int value() const { return _value; }
	bool is_valid() const { return (_value >= 1) && (_value <= 127); }
};


SCOPED_ENUM_DECLARE_BEGIN(NmtState)
{
	initializing = 0x00,
	stopped = 0x04,
	operational = 0x05,
	pre_operational = 0x7F
}
SCOPED_ENUM_DECLARE_END(NmtState)


SCOPED_ENUM_DECLARE_BEGIN(CobType)
{
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
}
SCOPED_ENUM_DECLARE_END(CobType)


const size_t cob_type_count = 16;


const emb::Array<uint32_t, cob_type_count> cob_function_codes =
{
	0x000,	// DUMMY
	0x000,	// NMT
	0x080,	// SYNC
	0x080,	// EMCY
	0x100,	// TIME
	0x180,	// TPDO1
	0x200,	// RPDO1
	0x280,	// TPDO2
	0x300,	// RPDO2
	0x380,	// TPDO3
	0x400,	// RPDO3
	0x480,	// TPDO4
	0x500,	// RPDO4
	0x580,	// TSDO
	0x600,	// RSDO
	0x700	// HEARTBEAT
};


inline uint32_t calculate_cob_id(CobType cob_type, NodeId node_id)
{
	if ((cob_type == CobType::nmt)
			|| (cob_type == CobType::sync)
			|| (cob_type == CobType::time))
	{
		return cob_function_codes[cob_type.underlying_value()];
	}
	return cob_function_codes[cob_type.underlying_value()] + node_id.value();
}


const emb::Array<unsigned int, cob_type_count> cob_sizes =
{
	0,	// DUMMY
	2,	// NMT
	0,	// SYNC
	2,	// EMCY
	6,	// TIME
	8,	// TPDO1
	8,	// RPDO1
	8,	// TPDO2
	8,	// RPDO2
	8,	// TPDO3
	8,	// RPDO3
	8,	// TPDO4
	8,	// RPDO4
	8,	// TSDO
	8,	// RSDO
	1	// HEARTBEAT
};


SCOPED_ENUM_DECLARE_BEGIN(TpdoType)
{
	tpdo1,
	tpdo2,
	tpdo3,
	tpdo4,
}
SCOPED_ENUM_DECLARE_END(TpdoType)


inline CobType to_cob_type(TpdoType tpdo_type)
{
	return static_cast<CobType>(static_cast<unsigned int>(CobType::tpdo1) + 2 * tpdo_type.underlying_value());
}


SCOPED_ENUM_DECLARE_BEGIN(RpdoType)
{
	rpdo1,
	rpdo2,
	rpdo3,
	rpdo4,
}
SCOPED_ENUM_DECLARE_END(RpdoType)


inline CobType to_cob_type(RpdoType rpdo_type)
{
	return static_cast<CobType>(static_cast<unsigned int>(CobType::rpdo1) + 2 * rpdo_type.underlying_value());
}


union ExpeditedSdoData
{
	int32_t i32;
	uint32_t u32;
	float f32;
};


struct ExpeditedSdo
{
	uint32_t data_size_indicated : 1;
	uint32_t expedited_transfer : 1;
	uint32_t data_empty_bytes : 2;
	uint32_t _reserved : 1;
	uint32_t cs : 3;
	uint32_t index : 16;
	uint32_t subindex : 8;
	ExpeditedSdoData data;
	ExpeditedSdo() { memset(this, 0, sizeof(ExpeditedSdo)); }
	ExpeditedSdo(uint64_t raw_msg) { memcpy(this, &raw_msg, sizeof(ExpeditedSdo)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(ExpeditedSdo));
		return data;
	}
};


namespace sdo_cs_codes {
const uint32_t ccs_init_write = 1;
const uint32_t scs_init_write = 3;
const uint32_t ccs_init_read = 2;
const uint32_t scs_init_read = 2;
}


SCOPED_ENUM_DECLARE_BEGIN(ODAccessStatus)
{
	success = 0,
	fail = 1,
	no_access = 2
}
SCOPED_ENUM_DECLARE_END(ODAccessStatus)


enum ODEntryDataType
{
	OD_BOOL,
	OD_INT16,
	OD_INT32,
	OD_UINT16,
	OD_UINT32,
	OD_FLOAT32,
	OD_ENUM16,
	OD_EXEC,
	OD_STRING_4CHARS
};


enum ODEntryAccessPermission
{
	OD_ACCESS_RW,
	OD_ACCESS_RO,
	OD_ACCESS_WO,
};


const size_t od_entry_data_sizes[9] = {sizeof(bool), sizeof(int16_t), sizeof(int32_t),
		sizeof(uint16_t), sizeof(uint32_t), sizeof(float), sizeof(uint16_t), 0, 0};


struct ODEntryKey
{
	uint32_t index;
	uint32_t subindex;
};


struct ODEntryValue
{
	const char* category;
	const char* subcategory;
	const char* name;
	const char* unit;
	ODEntryDataType data_type;
	ODEntryAccessPermission access_permission;
	uint32_t* data_ptr;
	ODAccessStatus (*read_func)(ExpeditedSdoData& retval);
	ODAccessStatus (*write_func)(ExpeditedSdoData val);
};


struct ODEntry
{
	ODEntryKey key;
	ODEntryValue value;

	bool has_read_permission() const
	{
		return (value.access_permission == OD_ACCESS_RW) || (value.access_permission == OD_ACCESS_RO);
	}

	bool has_write_permission() const
	{
		return (value.access_permission == OD_ACCESS_RW) || (value.access_permission == OD_ACCESS_WO);
	}
};


inline bool operator<(const ODEntry& lhs, const ODEntry& rhs)
{
	return (lhs.key.index < rhs.key.index)
			|| ((lhs.key.index == rhs.key.index) && (lhs.key.subindex < rhs.key.subindex));
}


struct ODEntryKeyAux
{
	uint32_t index;
	uint32_t subindex;
	ODEntryKeyAux(uint32_t _index, uint32_t _subindex) : index(_index), subindex(_subindex) {}
};


inline bool operator<(const ODEntryKeyAux& lhs, const ODEntry& rhs)
{
	return (lhs.index < rhs.key.index)
			|| ((lhs.index == rhs.key.index) && (lhs.subindex < rhs.key.subindex));
}


inline bool operator==(const ODEntryKeyAux& lhs, const ODEntry& rhs)
{
	return (lhs.index == rhs.key.index) && (lhs.subindex == rhs.key.subindex);
}


// Used in OD-entries which doesn't have direct access to data through pointer.
#define OD_NO_DIRECT_ACCESS static_cast<uint32_t*>(NULL)


// Used in OD-entries which have direct access to data through pointer.
#define OD_PTR(ptr) reinterpret_cast<uint32_t*>(ptr)


// Used in OD-entries which don't have read access to data through function.
inline ODAccessStatus OD_NO_INDIRECT_READ_ACCESS(ExpeditedSdoData& retval) { return ODAccessStatus::no_access; }


// Used in OD-entries which don't have write access to data through function.
inline ODAccessStatus OD_NO_INDIRECT_WRITE_ACCESS(ExpeditedSdoData val) { return ODAccessStatus::no_access; }


SCOPED_ENUM_DECLARE_BEGIN(ODExecStatus)
{
	success = 0,
	fail = 1,
	in_progress = 2,
	started = 3
}
SCOPED_ENUM_DECLARE_END(ODExecStatus)

} // namespace ucanopen

