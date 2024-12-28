#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>

namespace ucanopen {

impl::Server::Server(ucan::Module& can_module,
                     NodeId node_id,
                     ODEntry* object_dictionary,
                     size_t object_dictionary_size)
        : node_id_(node_id),
          can_module_(can_module),
          dictionary_(object_dictionary),
          dictionary_size_(object_dictionary_size) {
    nmt_state_ = NmtState::initializing;
    init_object_dictionary();
}

void impl::Server::init_object_dictionary() {
    assert(dictionary_ != nullptr);

    std::sort(dictionary_, dictionary_ + dictionary_size_);

    // Check OBJECT DICTIONARY correctness
    for (size_t i = 0; i < dictionary_size_; ++i) {
        // OD is sorted
        if (i < (dictionary_size_ - 1)) {
            assert(dictionary_[i] < dictionary_[i + 1]);
        }

        for (size_t j = i + 1; j < dictionary_size_; ++j) {
            // no od-entries with equal {index, subinex}
            assert((dictionary_[i].key.index != dictionary_[j].key.index) ||
                   (dictionary_[i].key.subindex !=
                    dictionary_[j].key.subindex));

            // no od-entries with equal {category, subcategory, name}
            [[maybe_unused]] bool categoryEqual =
                    ((strcmp(dictionary_[i].object.category,
                             dictionary_[j].object.category) == 0) ?
                             true :
                             false);
            [[maybe_unused]] bool subcategoryEqual =
                    ((strcmp(dictionary_[i].object.subcategory,
                             dictionary_[j].object.subcategory) == 0) ?
                             true :
                             false);
            [[maybe_unused]] bool nameEqual =
                    ((strcmp(dictionary_[i].object.name,
                             dictionary_[j].object.name) == 0) ?
                             true :
                             false);
            assert(!categoryEqual || !subcategoryEqual || !nameEqual);
        }

        if (dictionary_[i].object.has_read_permission()) {
            assert((dictionary_[i].object.read_func !=
                    OD_NO_INDIRECT_READ_ACCESS) ||
                   (dictionary_[i].object.ptr != OD_NO_DIRECT_ACCESS));
        }

        if (dictionary_[i].object.has_write_permission()) {
            assert(dictionary_[i].object.write_func !=
                           OD_NO_INDIRECT_WRITE_ACCESS ||
                   (dictionary_[i].object.ptr != OD_NO_DIRECT_ACCESS));
        }
    }
}

} // namespace ucanopen

#endif
#endif
