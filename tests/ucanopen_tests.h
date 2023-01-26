#pragma once


#include <ucanopen_c28x/server/ucanopen_server.h>
#include "sys/sysinfo/sysinfo.h"
#include "auto-generated/git_version.h"


namespace ucanopen {

namespace tests {

struct CobTpdo1
{
	uint64_t clock;
	CobTpdo1()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo1) == 4);
		memset(this, 0, sizeof(CobTpdo1));
	}
};


struct CobTpdo2
{
	uint32_t milliseconds;
	uint32_t seconds;
	CobTpdo2()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo2) == 4);
		memset(this, 0, sizeof(CobTpdo2));
	}
};


struct CobTpdo3
{
	uint16_t value_from_rpdo1 : 16;
	uint16_t value_from_rpdo2 : 16;
	uint16_t value_from_rpdo3 : 16;
	uint16_t value_from_rpdo4 : 16;
	CobTpdo3()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo3) == 4);
		memset(this, 0, sizeof(CobTpdo3));
	}
};


struct CobTpdo4
{
	uint64_t counter : 2;
	uint64_t errors : 31;
	uint64_t warnings : 31;
	CobTpdo4()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo4) == 4);
		memset(this, 0, sizeof(CobTpdo4));
	}
};


struct CobRpdo1
{
	uint32_t counter : 2;
	uint32_t _reserved : 30;
	float value;
	CobRpdo1()
	{
		EMB_STATIC_ASSERT(sizeof(CobRpdo1) == 4);
		memset(this, 0, sizeof(CobRpdo1));
	}
};


struct CobRpdo2
{
	uint32_t counter : 2;
	uint32_t _reserved : 30;
	float value;
	CobRpdo2()
	{
		EMB_STATIC_ASSERT(sizeof(CobRpdo2) == 4);
		memset(this, 0, sizeof(CobRpdo2));
	}
};


struct CobRpdo3
{
	uint32_t counter : 2;
	uint32_t _reserved : 30;
	float value;
	CobRpdo3()
	{
		EMB_STATIC_ASSERT(sizeof(CobRpdo3) == 4);
		memset(this, 0, sizeof(CobRpdo3));
	}
};


struct CobRpdo4
{
	uint32_t counter : 2;
	uint32_t _reserved : 30;
	float value;
	CobRpdo4()
	{
		EMB_STATIC_ASSERT(sizeof(CobRpdo4) == 4);
		memset(this, 0, sizeof(CobRpdo4));
	}
};


struct Object
{
	float value_from_rpdo1;
	float value_from_rpdo2;
	float value_from_rpdo3;
	float value_from_rpdo4;
	Object()
	{
		value_from_rpdo1 = 0;
		value_from_rpdo2 = 0;
		value_from_rpdo3 = 0;
		value_from_rpdo4 = 0;
	}
};
extern unsigned char object_alloc[sizeof(Object)];


extern ODEntry object_dictionary[];
extern const size_t object_dictionary_size;


class Server : public ucanopen::Server
{
private:
	static Object* _object;
public:
	Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
			NodeId node_id, mcu::can::Module* can_module,
			Object* object)
		: ucanopen::Server(mcu::ipc::traits::singlecore(), mcu::ipc::traits::primary(), ipc_flags,
				node_id, can_module, object_dictionary, object_dictionary_size)
	{
		_object = object;

		this->tpdo_service->registerTpdo(TpdoType::tpdo1, 50, _create_tpdo1);
		this->tpdo_service->registerTpdo(TpdoType::tpdo2, 50, _create_tpdo2);
		this->tpdo_service->registerTpdo(TpdoType::tpdo3, 100, _create_tpdo3);
		this->tpdo_service->registerTpdo(TpdoType::tpdo4, 100, _create_tpdo4);

		this->rpdo_service->register_rpdo(RpdoType::rpdo1, 1000, 0x182);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo1, _handle_rpdo1);
		this->rpdo_service->register_rpdo(RpdoType::rpdo2, 1000, 0x282);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo2, _handle_rpdo2);
		this->rpdo_service->register_rpdo(RpdoType::rpdo3, 1000);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo3, _handle_rpdo3);
		this->rpdo_service->register_rpdo(RpdoType::rpdo4, 1000);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo4, _handle_rpdo4);
	}

	Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
			NodeId node_id, mcu::can::Module* can_module,
			Object* object)
		: ucanopen::Server(mcu::ipc::traits::dualcore(), mcu::ipc::traits::primary(), ipc_flags,
				node_id, can_module)
	{
		_object = object;

		this->tpdo_service->registerTpdo(TpdoType::tpdo1, 50, _create_tpdo1);
		this->tpdo_service->registerTpdo(TpdoType::tpdo2, 50, _create_tpdo2);
		this->tpdo_service->registerTpdo(TpdoType::tpdo3, 100, _create_tpdo3);
		this->tpdo_service->registerTpdo(TpdoType::tpdo4, 100, _create_tpdo4);

		this->rpdo_service->register_rpdo(RpdoType::rpdo1, 1000, 0x182);
		this->rpdo_service->register_rpdo(RpdoType::rpdo2, 1000, 0x282);
		this->rpdo_service->register_rpdo(RpdoType::rpdo3, 1000);
		this->rpdo_service->register_rpdo(RpdoType::rpdo4, 1000);
	}

	Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary, const IpcFlags& ipc_flags,
			mcu::can::Peripheral can_peripheral, Object* object)
		: ucanopen::Server(mcu::ipc::traits::dualcore(), mcu::ipc::traits::secondary(), ipc_flags, can_peripheral,
				object_dictionary, object_dictionary_size)
	{
		_object = object;

		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo1, _handle_rpdo1);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo2, _handle_rpdo2);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo3, _handle_rpdo3);
		this->rpdo_service->register_rpdo_handler(RpdoType::rpdo4, _handle_rpdo4);
	}

protected:
	static can_payload _create_tpdo1()
	{
		CobTpdo1 tpdo;

		tpdo.clock = mcu::chrono::system_clock::now();

		return to_payload<CobTpdo1>(tpdo);
	}

	static can_payload _create_tpdo2()
	{
		CobTpdo2 tpdo;

		tpdo.seconds = mcu::chrono::system_clock::now() / 1000;
		tpdo.milliseconds = mcu::chrono::system_clock::now() - 1000 * tpdo.seconds;

		return to_payload<CobTpdo2>(tpdo);
	}

	static can_payload _create_tpdo3()
	{
		CobTpdo3 tpdo;

		tpdo.value_from_rpdo1 = _object->value_from_rpdo1;
		tpdo.value_from_rpdo2 = _object->value_from_rpdo2;
		tpdo.value_from_rpdo3 = _object->value_from_rpdo3;
		tpdo.value_from_rpdo4 = _object->value_from_rpdo4;

		return to_payload<CobTpdo3>(tpdo);
	}

	static can_payload _create_tpdo4()
	{
		static unsigned int counter = 0;
		CobTpdo4 tpdo;

		tpdo.counter = counter;
		tpdo.errors = syslog::errors();
		tpdo.warnings = syslog::warnings();

		counter = (counter + 1) % 4;
		return to_payload<CobTpdo4>(tpdo);
	}

	static void _handle_rpdo1(const can_payload& payload)
	{
		CobRpdo1 rpdo = from_payload<CobRpdo1>(payload);
		_object->value_from_rpdo1 = rpdo.value;
	}

	static void _handle_rpdo2(const can_payload& payload)
	{
		CobRpdo2 rpdo = from_payload<CobRpdo2>(payload);
		_object->value_from_rpdo2 = rpdo.value;
	}

	static void _handle_rpdo3(const can_payload& payload)
	{
		CobRpdo3 rpdo = from_payload<CobRpdo3>(payload);
		_object->value_from_rpdo3 = rpdo.value;
	}

	static void _handle_rpdo4(const can_payload& payload)
	{
		CobRpdo4 rpdo = from_payload<CobRpdo4>(payload);
		_object->value_from_rpdo4 = rpdo.value;
	}

	virtual void on_run()
	{
		static bool warning_detected = false;
		static uint64_t warning_timepoint = 0;

		if (syslog::has_warning(sys::Warning::can_bus_connection_lost))
		{
			if (!warning_detected)
			{
				warning_detected = true;
				warning_timepoint = mcu::chrono::system_clock::now();
			}

			if (mcu::chrono::system_clock::now() > warning_timepoint + 5000)
			{
				syslog::set_error(sys::Error::can_bus_connection_lost);
			}
		}
		else
		{
			warning_detected = false;
		}
	}
};

} // namespace tests

} // namespace ucanopen


