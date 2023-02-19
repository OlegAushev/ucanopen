#include <c28x_ucanopen/server/services/ucanopen_server_sdo.h>


namespace ucanopen {

unsigned char ServerSdoService::cana_rsdo_dualcore_alloc[sizeof(can_payload)]
		__attribute__((section("shared_ucanopen_cana_rsdo_data"), retain));
unsigned char ServerSdoService::canb_rsdo_dualcore_alloc[sizeof(can_payload)]
		__attribute__((section("shared_ucanopen_canb_rsdo_data"), retain));

unsigned char ServerSdoService::cana_tsdo_dualcore_alloc[sizeof(can_payload)]
		__attribute__((section("shared_ucanopen_cana_tsdo_data"), retain));
unsigned char ServerSdoService::canb_tsdo_dualcore_alloc[sizeof(can_payload)]
		__attribute__((section("shared_ucanopen_canb_tsdo_data"), retain));


ServerSdoService::ServerSdoService(impl::Server* server, const IpcFlags& ipc_flags)
	: _server(server)
{
	_rsdo_flag = ipc_flags.rsdo_received;
	_tsdo_flag = ipc_flags.tsdo_ready;

	switch (_server->_ipc_mode.underlying_value())
	{
		case mcu::ipc::Mode::singlecore:
			_rsdo_data = new can_payload;
			_tsdo_data = new can_payload;
			break;
		case mcu::ipc::Mode::dualcore:
			switch (_server->_can_peripheral.native_value())
			{
				case mcu::can::Peripheral::cana:
					_rsdo_data = new(cana_rsdo_dualcore_alloc) can_payload;
					_tsdo_data = new(cana_tsdo_dualcore_alloc) can_payload;
					break;
				case mcu::can::Peripheral::canb:
					_rsdo_data = new(canb_rsdo_dualcore_alloc) can_payload;
					_tsdo_data = new(canb_tsdo_dualcore_alloc) can_payload;
					break;
			}
			break;
	}
}


void ServerSdoService::handle_received()
{
	assert(_server->_ipc_mode == mcu::ipc::Mode::singlecore || _server->_ipc_role == mcu::ipc::Role::secondary);

	if (!_rsdo_flag.is_set())
	{
		return; // no RSDO received
	}

	ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(*_rsdo_data);
	_rsdo_flag.reset();
	if (rsdo.cs == sdo_cs_codes::abort)
	{
		return;
	}

	ExpeditedSdo tsdo;
	SdoAbortCode abort_code = SdoAbortCode::general_error;
	ODEntry* dictionary_end = _server->_dictionary + _server->_dictionary_size;
	ODObjectKey key = {rsdo.index, rsdo.subindex};

	const ODEntry* od_entry = emb::binary_find(_server->_dictionary, dictionary_end, key);

	if (od_entry == dictionary_end)
	{
		abort_code = SdoAbortCode::no_object;
	}
	else if (rsdo.cs == sdo_cs_codes::client_init_read)
	{
		abort_code = _read_expedited(od_entry, tsdo, rsdo);
	}
	else if (rsdo.cs == sdo_cs_codes::client_init_write)
	{
		abort_code = _write_expedited(od_entry, tsdo, rsdo);
	}
	else
	{
		abort_code = SdoAbortCode::invalid_cs;
	}

	switch (abort_code.native_value())
	{
		case SdoAbortCode::no_error:
			to_payload<ExpeditedSdo>(*_tsdo_data, tsdo);
			break;
		default:
			AbortSdo abort_tsdo;
			abort_tsdo.index = rsdo.index;
			abort_tsdo.subindex = rsdo.subindex;
			abort_tsdo.error_code = abort_code.underlying_value();
			to_payload<AbortSdo>(*_tsdo_data, abort_tsdo);
			break;
	}

	_tsdo_flag.local.set();
}


SdoAbortCode ServerSdoService::_read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo)
{
	if (!od_entry->object.has_read_permission())
	{
		return SdoAbortCode::read_access_wo;
	}

	SdoAbortCode abort_code;
	if (od_entry->object.has_direct_access())
	{
		memcpy(&tsdo.data.u32, od_entry->object.ptr, od_object_type_sizes[od_entry->object.type]);
		abort_code = SdoAbortCode::no_error;
	}
	else
	{
		abort_code = od_entry->object.read_func(tsdo.data);
	}

	if (abort_code == SdoAbortCode::no_error)
	{
		tsdo.index = rsdo.index;
		tsdo.subindex = rsdo.subindex;
		tsdo.cs = sdo_cs_codes::server_init_read;
		tsdo.expedited_transfer = 1;
		tsdo.data_size_indicated = 1;
		tsdo.data_empty_bytes = 4 - 2 * od_object_type_sizes[od_entry->object.type];
	}
	return abort_code;
}


SdoAbortCode ServerSdoService::_write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo)
{
	if (!od_entry->object.has_write_permission())
	{
		return SdoAbortCode::write_access_ro;
	}

	SdoAbortCode abort_code;
	if (od_entry->object.has_direct_access())
	{
		memcpy(od_entry->object.ptr, &rsdo.data.u32, od_object_type_sizes[od_entry->object.type]);
		abort_code = SdoAbortCode::no_error;
	}
	else
	{
		abort_code = od_entry->object.write_func(rsdo.data);
	}

	if (abort_code == SdoAbortCode::no_error)
	{
		tsdo.index = rsdo.index;
		tsdo.subindex = rsdo.subindex;
		tsdo.cs = sdo_cs_codes::server_init_write;
	}
	return abort_code;
}

} // namespace ucanopen

