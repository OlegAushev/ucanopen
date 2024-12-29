#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_block_service/fsm/fsm.hpp>

namespace ucanopen {
namespace blk_fsm {

AbstractState* AbstractState::create(State state) {
    switch (state) {
    case State::idle:
        return new IdleState;
    case State::download:
        return new DownloadState;
    case State::download_end:
        return new DownloadEndState;
    }
    return nullptr;
}

void AbstractState::destroy(State state, AbstractState* stateobj) {
    delete stateobj;
}

void IdleState::handle(SdoBlockService* _service,
                               const can_payload& payload) {}

void DownloadState::handle(SdoBlockService* _service,
                                   const can_payload& payload) {}

void DownloadEndState::handle(SdoBlockService* _service,
                                      const can_payload& payload) {}

} // namespace blk_fsm
} // namespace ucanopen

#endif
#endif
