#pragma once


#include <c28x_emb/emb_algorithm.h>
#include <c28x_ucanopen/server/impl/ucanopen_impl_server.h>
#include <new>
#include "sys/syslog/syslog.h"


namespace ucanopen {

class ServerSdoService
{
private:
	impl::Server* const _server;

	mcu::ipc::Flag _rsdo_flag;
	mcu::ipc::Flag _tsdo_flag;
	can_payload* _rsdo_data;
	can_payload* _tsdo_data;
	static unsigned char cana_rsdo_dualcore_alloc[sizeof(can_payload)];
	static unsigned char canb_rsdo_dualcore_alloc[sizeof(can_payload)];
	static unsigned char cana_tsdo_dualcore_alloc[sizeof(can_payload)];
	static unsigned char canb_tsdo_dualcore_alloc[sizeof(can_payload)];
public:
	ServerSdoService(impl::Server* server, const IpcFlags& ipc_flags);
	void handle_received();

	void recv()
	{
		assert(_server->_ipc_role == mcu::ipc::Role::primary);

		if (_rsdo_flag.local.is_set() || _tsdo_flag.is_set())
		{
			syslog::set_warning(sys::Warning::can_bus_overrun);
			syslog::add_message(sys::Message::can_sdo_request_lost);
		}
		else
		{
			_server->_can_module->recv(CobType::rsdo, _rsdo_data->data);
			_rsdo_flag.local.set();
		}
	}

	void send()
	{
		assert(_server->_ipc_role == mcu::ipc::Role::primary);

		if (!_tsdo_flag.is_set()) return;
		_server->_can_module->send(CobType::tsdo, _tsdo_data->data, cob_sizes[CobType::tsdo]);
		_tsdo_flag.reset();
	}
};

} // namespace ucanopen

