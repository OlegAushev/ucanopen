#ifdef MCUDRV_STM32
#ifdef STM32F4xx


#include "impl_server.h"


namespace ucanopen {


impl::Server::Server(mcu::can::Module& can_module, NodeId node_id,
                     ODEntry* object_dictionary, size_t object_dictionary_size)
        : _node_id(node_id)
        , _can_module(can_module)
        , _dictionary(object_dictionary)
        , _dictionary_size(object_dictionary_size) {
    _nmt_state = NmtState::initializing;
    _init_object_dictionary();
}


void impl::Server::_init_object_dictionary() {
    assert(_dictionary != nullptr);

    std::sort(_dictionary, _dictionary + _dictionary_size);

        // Check OBJECT DICTIONARY correctness
    for (size_t i = 0; i < _dictionary_size; ++i) {
        // OD is sorted
        if (i < (_dictionary_size - 1)) {
            assert(_dictionary[i] < _dictionary[i+1]);
        }

        for (size_t j = i+1; j < _dictionary_size; ++j) {
            // no od-entries with equal {index, subinex}
            assert((_dictionary[i].key.index != _dictionary[j].key.index)
                || (_dictionary[i].key.subindex != _dictionary[j].key.subindex));

            // no od-entries with equal {category, subcategory, name}
            [[maybe_unused]] bool categoryEqual =
                    ((strcmp(_dictionary[i].object.category, _dictionary[j].object.category) == 0) ? true : false);
            [[maybe_unused]] bool subcategoryEqual =
                    ((strcmp(_dictionary[i].object.subcategory, _dictionary[j].object.subcategory) == 0) ? true : false);
            [[maybe_unused]] bool nameEqual =
                    ((strcmp(_dictionary[i].object.name, _dictionary[j].object.name) == 0) ? true : false);
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
    }
}


} // namespace ucanopen


#endif
#endif
