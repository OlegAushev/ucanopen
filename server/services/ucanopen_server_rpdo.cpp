#include <ucanopen_c28x/server/services/ucanopen_server_rpdo.h>


namespace ucanopen {

unsigned char ServerRpdoService::cana_rpdo_dualcore_alloc[sizeof(emb::Array<ServerRpdoService::Message, 4>)]
		__attribute__((section("shared_ucanopen_cana_rpdo_data"), retain));
unsigned char ServerRpdoService::canb_rpdo_dualcore_alloc[sizeof(emb::Array<ServerRpdoService::Message, 4>)]
		__attribute__((section("shared_ucanopen_canb_rpdo_data"), retain));


ServerRpdoService::ServerRpdoService(impl::Server* server, const IpcFlags& ipc_flags)
	: _server(server)
{
	switch (_server->_ipc_mode.underlying_value())
	{
		case mcu::ipc::Mode::singlecore:
			_rpdo_list = new emb::Array<Message, 4>;
			break;
		case mcu::ipc::Mode::dualcore:
			switch (_server->_can_peripheral.native_value())
			{
				case mcu::can::Peripheral::cana:
					_rpdo_list = new(cana_rpdo_dualcore_alloc) emb::Array<Message, 4>;
					break;
				case mcu::can::Peripheral::canb:
					_rpdo_list = new(canb_rpdo_dualcore_alloc) emb::Array<Message, 4>;
					break;
			}
			break;
	}

	for (size_t i = 0; i < _rpdo_list->size(); ++i)
	{
		(*_rpdo_list)[i].timeout = 0;
		(*_rpdo_list)[i].timepoint = mcu::chrono::system_clock::now();
	}

	for (size_t i = 0; i < _handlers.size(); ++i)
	{
		_handlers[i] = reinterpret_cast<void(*)(const can_payload& data)>(NULL);
	}

	_received_flags[RpdoType::rpdo1] = ipc_flags.rpdo1_received;
	_received_flags[RpdoType::rpdo2] = ipc_flags.rpdo2_received;
	_received_flags[RpdoType::rpdo3] = ipc_flags.rpdo3_received;
	_received_flags[RpdoType::rpdo4] = ipc_flags.rpdo4_received;
}


void ServerRpdoService::register_rpdo(RpdoType rpdo_type, uint64_t timeout, unsigned int id)
{
	assert(_server->_ipc_role == mcu::ipc::Role::primary);

	(*_rpdo_list)[rpdo_type.underlying_value()].timeout = timeout;
	if (id != 0)
	{
		CobType cob = to_cob_type(rpdo_type);
		_server->_message_objects[cob.underlying_value()].frame_id = id;
		_server->_can_module->setup_message_object(_server->_message_objects[cob.underlying_value()]);
	}
}


void ServerRpdoService::register_rpdo_handler(RpdoType rpdo_type, void (*handler)(const can_payload& data))
{
	assert(_server->_ipc_mode == mcu::ipc::Mode::singlecore || _server->_ipc_role == mcu::ipc::Role::secondary);

	_handlers[rpdo_type.underlying_value()] = handler;
}

} // namespace ucanopen

