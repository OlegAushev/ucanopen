#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include "sdo_block_service.h"


namespace ucanopen {
namespace blk {


SdoBlockService::SdoBlockService(impl::Server& server)
        : emb::fsm::abstract_object<State, fsm::AbstractState, state_count>(State::idle)
        , _server(server) {}


namespace fsm {


AbstractState* AbstractState::create(State state) {
    switch (state)
    {
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


void IdleState::handle_message(SdoBlockService* _service, const can_payload& payload) {

}


void DownloadState::handle_message(SdoBlockService* _service, const can_payload& payload) {

}


void DownloadEndState::handle_message(SdoBlockService* _service, const can_payload& payload) {

}




} // namespace fsm


} // namespace blk
} // namespace ucanopen


#endif
#endif
