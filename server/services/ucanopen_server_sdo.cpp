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

	if (!_rsdo_flag.is_set()) return;

	ODAccessStatus status = ODAccessStatus::no_access;
	ODEntry* dictionary_end = _server->_dictionary + _server->_dictionary_size;

	ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(*_rsdo_data);
	ExpeditedSdo tsdo;

	_rsdo_flag.reset();

	const ODEntry* od_entry = emb::binary_find(_server->_dictionary, dictionary_end,
			ODEntryKeyAux(rsdo.index, rsdo.subindex));

	if (od_entry == dictionary_end) return;	// OD-entry not found

	if (rsdo.cs == sdo_cs_codes::ccs_init_read)
	{
		if ((od_entry->value.data_ptr != OD_NO_DIRECT_ACCESS) && od_entry->has_read_permission())
		{
			memcpy(&tsdo.data.u32, od_entry->value.data_ptr, od_entry_data_sizes[od_entry->value.data_type]);
			status = ODAccessStatus::success;
		}
		else
		{
			status = od_entry->value.read_func(tsdo.data);
		}
	}
	else if (rsdo.cs == sdo_cs_codes::ccs_init_write)
	{
		if ((od_entry->value.data_ptr != OD_NO_DIRECT_ACCESS) && od_entry->has_write_permission())
		{
			memcpy(od_entry->value.data_ptr, &rsdo.data.u32, od_entry_data_sizes[od_entry->value.data_type]);
			status = ODAccessStatus::success;
		}
		else
		{
			status = od_entry->value.write_func(rsdo.data);
		}
	}
	else
	{
		return;
	}

	switch (status.underlying_value())
	{
		case ODAccessStatus::success:
			tsdo.index = rsdo.index;
			tsdo.subindex = rsdo.subindex;
			if (rsdo.cs == sdo_cs_codes::ccs_init_read)
			{
				tsdo.cs = sdo_cs_codes::scs_init_read;	// read/upload response
				tsdo.expedited_transfer = 1;
				tsdo.data_size_indicated = 1;
				tsdo.data_empty_bytes = 0;
			}
			else if (rsdo.cs == sdo_cs_codes::ccs_init_write)
			{
				tsdo.cs = sdo_cs_codes::scs_init_write;	// write/download response
				tsdo.expedited_transfer = 0;
				tsdo.data_size_indicated = 0;
				tsdo.data_empty_bytes = 0;
			}
			else
			{
				return;
			}
			to_payload<ExpeditedSdo>(*_tsdo_data, tsdo);
			_tsdo_flag.local.set();
			break;
		case ODAccessStatus::fail:
		case ODAccessStatus::no_access:
			return;
	}
}

} // namespace ucanopen

