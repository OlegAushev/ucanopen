#pragma once


#include <c28x_mcu/f2837xd/chrono/mcu_chrono.h>
#include <c28x_ucanopen/server/impl/ucanopen_impl_server.h>
#include <c28x_ucanopen/server/impl/ucanopen_impl_server.h>
#include <c28x_ucanopen/server/services/ucanopen_server_heartbeat.h>
#include <c28x_ucanopen/server/services/ucanopen_server_rpdo.h>
#include <c28x_ucanopen/server/services/ucanopen_server_sdo.h>
#include <c28x_ucanopen/server/services/ucanopen_server_tpdo.h>
#include <c28x_ucanopen/ucanopen_def.h>
#include "sys/syslog/syslog.h"


namespace ucanopen {

namespace impl {

extern unsigned char cana_rsdo_dualcore_alloc[sizeof(can_payload)];
extern unsigned char canb_rsdo_dualcore_alloc[sizeof(can_payload)];

extern unsigned char cana_tsdo_dualcore_alloc[sizeof(can_payload)];
extern unsigned char canb_tsdo_dualcore_alloc[sizeof(can_payload)];

} // namespace impl


class Server : public impl::Server, public emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>
{
protected:
	ServerHeartbeatService* heartbeat_service;
	ServerTpdoService* tpdo_service;
	ServerRpdoService* rpdo_service;
	ServerSdoService* sdo_service;

	virtual void on_run() {}
public:
	Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
			NodeId node_id, mcu::can::Module* can_module,
			ODEntry* object_dictionary, size_t object_dictionary_size);

	Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
			NodeId node_id, mcu::can::Module* can_module);

	Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary, const IpcFlags& ipc_flags,
			mcu::can::Peripheral can_peripheral, ODEntry* object_dictionary, size_t object_dictionary_size);

	void enable()
	{
		if (this->_can_module)
		{
			this->_can_module->enable_interrupts();
			this->_nmt_state = NmtState::operational;
		}
	}

	void disable()
	{
		if (this->_can_module)
		{
			this->_can_module->disable_interrupts();
			this->_nmt_state = NmtState::stopped;
		}
	}

	void run()
	{
		switch (this->_ipc_mode.underlying_value())
		{
			case mcu::ipc::Mode::singlecore:
				heartbeat_service->send();
				tpdo_service->send();
				rpdo_service->handle_received();
				sdo_service->handle_received();
				sdo_service->send();
				rpdo_service->check_connection();
				on_run();
				break;
			case mcu::ipc::Mode::dualcore:
				switch (this->_ipc_role.underlying_value())
				{
					case mcu::ipc::Role::primary:
						heartbeat_service->send();
						tpdo_service->send();
						sdo_service->send();
						rpdo_service->check_connection();
						break;
					case mcu::ipc::Role::secondary:
						rpdo_service->handle_received();
						sdo_service->handle_received();
						on_run();
						break;
				}
				break;
		}
	}

private:
	template <mcu::can::Peripheral::enum_type Periph>
	static void on_frame_received(mcu::can::Module* can_module, uint32_t interrupt_cause, uint16_t status)
	{
		Server* server = Server::instance(Periph);

		switch (interrupt_cause)
		{
			case CAN_INT_INT0ID_STATUS:
				switch (status)
				{
					case CAN_STATUS_PERR:
					case CAN_STATUS_BUS_OFF:
					case CAN_STATUS_EWARN:
					case CAN_STATUS_LEC_BIT1:
					case CAN_STATUS_LEC_BIT0:
					case CAN_STATUS_LEC_CRC:
						syslog::set_warning(sys::Warning::can_bus_error);
						break;
					default:
						break;
				}
				break;
			case CobType::rpdo1:
			case CobType::rpdo2:
			case CobType::rpdo3:
			case CobType::rpdo4:
			{
				syslog::reset_warning(sys::Warning::can_bus_error);
				server->rpdo_service->recv(CobType(interrupt_cause));
				break;
			}
			case CobType::rsdo:
			{
				syslog::reset_warning(sys::Warning::can_bus_error);
				server->sdo_service->recv();
				break;
			}
			default:
				break;
		}
	}
};

} // namespace ucanopen

