#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_block_service/sdo_block_service.hpp>

namespace ucanopen {

SdoBlockService::SdoBlockService(impl::Server& server)
        : emb::fsm::abstract_object<blk_fsm::State,
                                    blk_fsm::AbstractState,
                                    blk_fsm::state_count>(blk_fsm::State::idle),
          server_(server),
          block_(std::make_unique<PayloadBlock>()) {}

} // namespace ucanopen

#endif
#endif
