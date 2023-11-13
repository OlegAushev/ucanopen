#ifdef MCUDRV_C28X


#include "impl_server.h"


namespace ucanopen {


impl::Server::Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary,
                     mcu::can::Module* can_module, NodeId node_id,
                     ODEntry* object_dictionary, int object_dictionary_size)
        : _ipc_mode(mcu::ipc::Mode::singlecore)
        , _ipc_role(mcu::ipc::Role::primary)
        , _node_id(node_id)
        , _can_module(can_module)
        , _dictionary(object_dictionary)
        , _dictionary_size(object_dictionary_size) {
    _can_peripheral = _can_module->peripheral();
    _nmt_state = NmtState::initializing;
    _init_message_objects();
    _init_object_dictionary();
}


impl::Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary,
                     mcu::can::Module* can_module, NodeId node_id)
        : _ipc_mode(mcu::ipc::Mode::dualcore)
        , _ipc_role(mcu::ipc::Role::primary)
        , _node_id(node_id)
        , _can_module(can_module)
        , _dictionary(static_cast<ODEntry*>(NULL))
        , _dictionary_size(0) {
    _can_peripheral = _can_module->peripheral();
    _nmt_state = NmtState::initializing;
    _init_message_objects();
}


impl::Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary,
                     mcu::can::Peripheral can_peripheral, ODEntry* object_dictionary, int object_dictionary_size)
        : _ipc_mode(mcu::ipc::Mode::dualcore)
        , _ipc_role(mcu::ipc::Role::secondary)
        , _node_id(NodeId(0))
        , _can_module(static_cast<mcu::can::Module*>(NULL))
        , _dictionary(object_dictionary)
        , _dictionary_size(object_dictionary_size) {
    _can_peripheral = can_peripheral;
    _nmt_state = NmtState::initializing;
    _init_object_dictionary();
}


void impl::Server::_init_message_objects() {
    for (size_t i = 0; i < cob_count; ++i) {
        _message_objects[i].obj_id = i;
        _message_objects[i].frame_id = calculate_cob_id(Cob(i), this->_node_id);
        _message_objects[i].frame_type = CAN_MSG_FRAME_STD;
        _message_objects[i].frame_idmask = 0;
        _message_objects[i].data_len = cob_data_len[i];
    }

    _message_objects[Cob::emcy].obj_type
            = _message_objects[Cob::tpdo1].obj_type
            = _message_objects[Cob::tpdo2].obj_type
            = _message_objects[Cob::tpdo3].obj_type
            = _message_objects[Cob::tpdo4].obj_type
            = _message_objects[Cob::tsdo].obj_type
            = _message_objects[Cob::heartbeat].obj_type
            = CAN_MSG_OBJ_TYPE_TX;

    _message_objects[Cob::nmt].obj_type
            = _message_objects[Cob::sync].obj_type
            = _message_objects[Cob::time].obj_type
            = _message_objects[Cob::rpdo1].obj_type
            = _message_objects[Cob::rpdo2].obj_type
            = _message_objects[Cob::rpdo3].obj_type
            = _message_objects[Cob::rpdo4].obj_type
            = _message_objects[Cob::rsdo].obj_type
            = CAN_MSG_OBJ_TYPE_RX;

    _message_objects[Cob::emcy].flags
            = _message_objects[Cob::tpdo1].flags
            = _message_objects[Cob::tpdo2].flags
            = _message_objects[Cob::tpdo3].flags
            = _message_objects[Cob::tpdo4].flags
            = _message_objects[Cob::tsdo].flags
            = _message_objects[Cob::heartbeat].flags
            = CAN_MSG_OBJ_NO_FLAGS;

    _message_objects[Cob::nmt].flags
            = _message_objects[Cob::sync].flags
            = _message_objects[Cob::time].flags
            = _message_objects[Cob::rpdo1].flags
            = _message_objects[Cob::rpdo2].flags
            = _message_objects[Cob::rpdo3].flags
            = _message_objects[Cob::rpdo4].flags
            = _message_objects[Cob::rsdo].flags
            = CAN_MSG_OBJ_RX_INT_ENABLE;

    for (int i = 1; i < cob_count; ++i) {
        // count from 1 - skip dummy COB
        this->_can_module->setup_message_object(_message_objects[i]);
    }
}


void impl::Server::_init_object_dictionary() {
    assert(_dictionary != NULL);

    std::sort(_dictionary, _dictionary + _dictionary_size);

    // Check OBJECT DICTIONARY correctness
    for (size_t i = 0; i < _dictionary_size; ++i) {
        // OD is sorted
        if (i < (_dictionary_size - 1)) {
            assert(_dictionary[i] < _dictionary[i+1]);
        }

        for (int j = i+1; j < _dictionary_size; ++j) {
            // no od-entries with equal {index, subinex}
            assert((_dictionary[i].key.index != _dictionary[j].key.index)
                || (_dictionary[i].key.subindex != _dictionary[j].key.subindex));

            // no od-entries with equal {category, subcategory, name}
            bool categoryEqual = ((strcmp(_dictionary[i].object.category, _dictionary[j].object.category) == 0) ? true : false);
            bool subcategoryEqual = ((strcmp(_dictionary[i].object.subcategory, _dictionary[j].object.subcategory) == 0) ? true : false);
            bool nameEqual = ((strcmp(_dictionary[i].object.name, _dictionary[j].object.name) == 0) ? true : false);
            assert(!categoryEqual || !subcategoryEqual || !nameEqual);
        }

        if (_dictionary[i].object.has_read_permission()) {
            assert((_dictionary[i].object.read_func != OD_NO_INDIRECT_READ_ACCESS)
                || (_dictionary[i].object.ptr != OD_NO_DIRECT_ACCESS));
        }

        if (_dictionary[i].object.has_write_permission()) {
            assert(_dictionary[i].object.write_func != OD_NO_INDIRECT_WRITE_ACCESS
               || (_dictionary[i].object.ptr != OD_NO_DIRECT_ACCESS));
        }

        if (_dictionary[i].object.default_value.has_value()) {
            ODObjectDataType data_type = _dictionary[i].object.data_type;
            assert(data_type != OD_EXEC && data_type != OD_STRING);
        }
    }
}


} // namespace ucanopen


#endif
