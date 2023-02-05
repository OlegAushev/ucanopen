#include <c28x_ucanopen/server/impl/ucanopen_impl_server.h>


namespace ucanopen {

impl::Server::Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary,
		NodeId node_id, mcu::can::Module* can_module,
		ODEntry* object_dictionary, size_t object_dictionary_size)
	: _ipc_mode(mcu::ipc::Mode::singlecore)
	, _ipc_role(mcu::ipc::Role::primary)
	, _node_id(node_id)
	, _can_module(can_module)
	, _dictionary(object_dictionary)
	, _dictionary_size(object_dictionary_size)
{
	_can_peripheral = _can_module->peripheral();
	_nmt_state = NmtState::initializing;
	_init_message_objects();
	_init_object_dictionary();
}


impl::Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary,
		NodeId node_id, mcu::can::Module* can_module)
	: _ipc_mode(mcu::ipc::Mode::dualcore)
	, _ipc_role(mcu::ipc::Role::primary)
	, _node_id(node_id)
	, _can_module(can_module)
	, _dictionary(static_cast<ODEntry*>(NULL))
	, _dictionary_size(0)
{
	_can_peripheral = _can_module->peripheral();
	_nmt_state = NmtState::initializing;
	_init_message_objects();
}


impl::Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary,
		mcu::can::Peripheral can_peripheral, ODEntry* object_dictionary, size_t object_dictionary_size)
	: _ipc_mode(mcu::ipc::Mode::dualcore)
	, _ipc_role(mcu::ipc::Role::secondary)
	, _node_id(NodeId(0))
	, _can_module(static_cast<mcu::can::Module*>(NULL))
	, _dictionary(object_dictionary)
	, _dictionary_size(object_dictionary_size)
{
	_can_peripheral = can_peripheral;
	_nmt_state = NmtState::initializing;
	_init_object_dictionary();
}


void impl::Server::_init_message_objects()
{
	for (size_t i = 0; i < cob_type_count; ++i)
	{
		_message_objects[i].obj_id = i;
		_message_objects[i].frame_id = calculate_cob_id(CobType(i), this->_node_id);
		_message_objects[i].frame_type = CAN_MSG_FRAME_STD;
		_message_objects[i].frame_idmask = 0;
		_message_objects[i].data_len = cob_sizes[i];
	}

	_message_objects[CobType::emcy].obj_type
			= _message_objects[CobType::tpdo1].obj_type
			= _message_objects[CobType::tpdo2].obj_type
			= _message_objects[CobType::tpdo3].obj_type
			= _message_objects[CobType::tpdo4].obj_type
			= _message_objects[CobType::tsdo].obj_type
			= _message_objects[CobType::heartbeat].obj_type
			= CAN_MSG_OBJ_TYPE_TX;

	_message_objects[CobType::nmt].obj_type
			= _message_objects[CobType::sync].obj_type
			= _message_objects[CobType::time].obj_type
			= _message_objects[CobType::rpdo1].obj_type
			= _message_objects[CobType::rpdo2].obj_type
			= _message_objects[CobType::rpdo3].obj_type
			= _message_objects[CobType::rpdo4].obj_type
			= _message_objects[CobType::rsdo].obj_type
			= CAN_MSG_OBJ_TYPE_RX;

	_message_objects[CobType::emcy].flags
			= _message_objects[CobType::tpdo1].flags
			= _message_objects[CobType::tpdo2].flags
			= _message_objects[CobType::tpdo3].flags
			= _message_objects[CobType::tpdo4].flags
			= _message_objects[CobType::tsdo].flags
			= _message_objects[CobType::heartbeat].flags
			= CAN_MSG_OBJ_NO_FLAGS;

	_message_objects[CobType::nmt].flags
			= _message_objects[CobType::sync].flags
			= _message_objects[CobType::time].flags
			= _message_objects[CobType::rpdo1].flags
			= _message_objects[CobType::rpdo2].flags
			= _message_objects[CobType::rpdo3].flags
			= _message_objects[CobType::rpdo4].flags
			= _message_objects[CobType::rsdo].flags
			= CAN_MSG_OBJ_RX_INT_ENABLE;

	for (size_t i = 1; i < cob_type_count; ++i)	// count from 1 - skip dummy COB
	{
		this->_can_module->setup_message_object(_message_objects[i]);
	}
}


void impl::Server::_init_object_dictionary()
{
	assert(_dictionary != NULL);

	std::sort(_dictionary, _dictionary + _dictionary_size);

	// Check OBJECT DICTIONARY correctness
	for (size_t i = 0; i < _dictionary_size; ++i)
	{
		// OD is sorted
		if (i < (_dictionary_size - 1))
		{
			assert(_dictionary[i] < _dictionary[i+1]);
		}

		for (size_t j = i+1; j < _dictionary_size; ++j)
		{
			// no od-entries with equal {index, subinex}
			assert((_dictionary[i].key.index != _dictionary[j].key.index)
				|| (_dictionary[i].key.subindex != _dictionary[j].key.subindex));

			// no od-entries with equal {category, subcategory, name}
			bool categoryEqual = ((strcmp(_dictionary[i].value.category, _dictionary[j].value.category) == 0) ? true : false);
			bool subcategoryEqual = ((strcmp(_dictionary[i].value.subcategory, _dictionary[j].value.subcategory) == 0) ? true : false);
			bool nameEqual = ((strcmp(_dictionary[i].value.name, _dictionary[j].value.name) == 0) ? true : false);
			assert(!categoryEqual || !subcategoryEqual || !nameEqual);
		}

		if (_dictionary[i].has_read_permission())
		{
			assert((_dictionary[i].value.read_func != OD_NO_INDIRECT_READ_ACCESS)
					|| (_dictionary[i].value.data_ptr != OD_NO_DIRECT_ACCESS));
		}
		else
		{
			assert(_dictionary[i].value.read_func == OD_NO_INDIRECT_READ_ACCESS
					&& (_dictionary[i].value.data_ptr == OD_NO_DIRECT_ACCESS));
		}

		if (_dictionary[i].has_write_permission())
		{
			assert(_dictionary[i].value.write_func != OD_NO_INDIRECT_WRITE_ACCESS
					|| (_dictionary[i].value.data_ptr != OD_NO_DIRECT_ACCESS));
		}
		else
		{
			assert(_dictionary[i].value.write_func == OD_NO_INDIRECT_WRITE_ACCESS
					&& (_dictionary[i].value.data_ptr == OD_NO_DIRECT_ACCESS));
		}
	}
}

} // namespace ucanopen

