#pragma once


#include "c28x_mcu_drivers/f2837xd/chrono/chrono.h"
#include "../impl/impl_server.h"
#include <new>
#include "sys/syslog/syslog.h"


namespace ucanopen {

class ServerRpdoService
{
private:
	impl::Server* const _server;

	struct Message
	{
		emb::chrono::milliseconds timeout;
		emb::chrono::milliseconds timepoint;
		can_payload payload;
	};
	emb::Array<Message, 4>* _rpdo_list;
	static unsigned char cana_rpdo_dualcore_alloc[sizeof(emb::Array<Message, 4>)];
	static unsigned char canb_rpdo_dualcore_alloc[sizeof(emb::Array<Message, 4>)];
	emb::Array<mcu::ipc::Flag, 4> _received_flags;
	emb::Array<void(*)(const can_payload& payload), 4> _handlers;
public:
	ServerRpdoService(impl::Server* server, const IpcFlags& ipc_flags);
	void register_rpdo(RpdoType rpdo_type, emb::chrono::milliseconds timeout, unsigned int id = 0);
	void register_rpdo_handler(RpdoType rpdo_type, void (*handler)(const can_payload& data));

	void recv(CobType cob_type)
	{
		assert(_server->_ipc_role == mcu::ipc::Role::primary);

		if (cob_type != CobType::rpdo1
				&& cob_type != CobType::rpdo2
				&& cob_type != CobType::rpdo3
				&& cob_type != CobType::rpdo4) return;

		RpdoType rpdo_type((cob_type.underlying_value() - static_cast<unsigned int>(CobType::rpdo1)) / 2);

		(*_rpdo_list)[rpdo_type.underlying_value()].timepoint = mcu::chrono::system_clock::now();
		if (_received_flags[rpdo_type.underlying_value()].local.is_set())
		{
			syslog::set_warning(sys::Warning::can_bus_overrun);
		}
		else
		{
			// there is no unprocessed RPDO of this type
			_server->_can_module->recv(cob_type.underlying_value(), (*_rpdo_list)[rpdo_type.underlying_value()].payload.data);
			_received_flags[rpdo_type.underlying_value()].local.set();
		}
	}

	void handle_received()
	{
		assert(_server->_ipc_mode == mcu::ipc::Mode::singlecore || _server->_ipc_role == mcu::ipc::Role::secondary);

		for (size_t i = 0; i < _rpdo_list->size(); ++i)
		{
			if (!_handlers[i]) continue;
			if (_received_flags[i].is_set())
			{
				_handlers[i]((*_rpdo_list)[i].payload);
				_received_flags[i].reset();
			}
		}
	}

	void check_connection()
	{
		assert(_server->_ipc_role == mcu::ipc::Role::primary);

		for (size_t i = 0; i < _rpdo_list->size(); ++i)
		{
			if ((*_rpdo_list)[i].timeout.count() <= 0) continue;

			if (mcu::chrono::system_clock::now() > (*_rpdo_list)[i].timepoint + (*_rpdo_list)[i].timeout)
			{
				syslog::set_warning(sys::Warning::can_bus_connection_lost);
				return;
			}
		}
		syslog::reset_warning(sys::Warning::can_bus_connection_lost);
		return;
	}
};

} // namespace ucanopen

