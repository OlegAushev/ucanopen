#include <c28x_ucanopen/server/services/ucanopen_server_tpdo.h>


namespace ucanopen {

ServerTpdoService::ServerTpdoService(impl::Server* server)
	: _server(server)
{
	for (size_t i = 0; i < _tpdo_list.size(); ++i)
	{
		_tpdo_list[i].period = emb::chrono::milliseconds(-1);
		_tpdo_list[i].timepoint = mcu::chrono::system_clock::now();
		_tpdo_list[i].creator = reinterpret_cast<can_payload(*)()>(NULL);
	}
}


void ServerTpdoService::registerTpdo(TpdoType tpdo_type, emb::chrono::milliseconds period, can_payload (*creator)())
{
	assert(_server->_ipc_role == mcu::ipc::Role::primary);

	_tpdo_list[tpdo_type.underlying_value()].period = period;
	_tpdo_list[tpdo_type.underlying_value()].creator = creator;
}

} // namespace ucanopen

