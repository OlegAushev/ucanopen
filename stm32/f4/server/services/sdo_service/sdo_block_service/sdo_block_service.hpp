#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>
#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_block_service/fsm/fsm.hpp>
#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_block_service/sdo_block_service_def.hpp>

#include <emblib/static_vector.hpp>
#include <memory>

namespace ucanopen {

using PayloadBlock = emb::static_vector<can_payload, 127>;

class SdoBlockConsumer {
public:
    virtual void process(const PayloadBlock& block) = 0;
};

class SdoBlockService final
        : public emb::fsm::abstract_object<blk_fsm::State,
                                           blk_fsm::AbstractState,
                                           blk_fsm::state_count> {
private:
    impl::Server& server_;
    SdoBlockConsumer* consumer_{nullptr};
    std::unique_ptr<PayloadBlock> block_;
public:
    SdoBlockService(impl::Server& server);
    void handle(const can_payload& payload) {
        current_state_->handle(this, payload);
    }
};

} // namespace ucanopen

#endif
#endif
