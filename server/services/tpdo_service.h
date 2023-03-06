#pragma once


#include "../impl/impl_server.h"
#include <c28x_mcu_drivers/f2837xd/chrono/chrono.h>



namespace ucanopen {

class ServerTpdoService
{
private:
	impl::Server* const _server;

	struct Message
	{
		emb::chrono::milliseconds period;
		emb::chrono::milliseconds timepoint;
		can_payload (*creator)();
	};
	emb::Array<Message, 4> _tpdo_list;
public:
	ServerTpdoService(impl::Server* server);
	void registerTpdo(TpdoType tpdo_type, emb::chrono::milliseconds period, can_payload (*creator)());

	void send()
	{
		assert(_server->_ipc_role == mcu::ipc::Role::primary);

		for (size_t i = 0; i < _tpdo_list.size(); ++i)
		{
			if (!_tpdo_list[i].creator) continue;
			if (mcu::chrono::system_clock::now() < _tpdo_list[i].timepoint + _tpdo_list[i].period) continue;

			can_payload payload = _tpdo_list[i].creator();
			CobType cob_type = to_cob_type(TpdoType(i));
			_server->_can_module->send(cob_type.underlying_value(), payload.data, cob_sizes[cob_type.underlying_value()]);
			_tpdo_list[i].timepoint = mcu::chrono::system_clock::now();
		}
	}
};

} // namespace ucanopen

