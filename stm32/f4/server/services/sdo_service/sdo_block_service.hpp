#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/impl/impl_server.hpp>
#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_block_service_def.hpp>

#include <emblib/fsm.hpp>

namespace ucanopen {
namespace blk {

class SdoBlockService;

constexpr size_t state_count = 3;
enum class State : unsigned int {
    idle,
    download,
    download_end,
};

namespace fsm {

class AbstractState : public emb::fsm::abstract_state<SdoBlockService, State> {
protected:
    AbstractState(State id)
            : emb::fsm::abstract_state<SdoBlockService, State>(id) {}
public:
    static AbstractState* create(State state);
    static void destroy(State state, AbstractState* stateobj);
    virtual ~AbstractState() {}

    virtual void handle_message(SdoBlockService* _service,
                                const can_payload& payload) = 0;
};

class IdleState final : public AbstractState {
protected:
    virtual void initiate(SdoBlockService* _service) override {}
    virtual void finalize(SdoBlockService* _service) override {}
public:
    IdleState() : AbstractState(State::idle) {}
    virtual void handle_message(SdoBlockService* _service,
                                const can_payload& payload) override;
};

class DownloadState final : public AbstractState {
protected:
    virtual void initiate(SdoBlockService* _service) override {}
    virtual void finalize(SdoBlockService* _service) override {}
public:
    DownloadState() : AbstractState(State::download) {}
    virtual void handle_message(SdoBlockService* _service,
                                const can_payload& payload) override;
};

class DownloadEndState final : public AbstractState {
protected:
    virtual void initiate(SdoBlockService* _service) override {}
    virtual void finalize(SdoBlockService* _service) override {}
public:
    DownloadEndState() : AbstractState(State::download_end) {}
    virtual void handle_message(SdoBlockService* _service,
                                const can_payload& payload) override;
};

} // namespace fsm

class SdoBlockService final
        : public emb::fsm::abstract_object<State,
                                           fsm::AbstractState,
                                           state_count> {
private:
    impl::Server& _server;
public:
    SdoBlockService(impl::Server& server);
    void handle_message(const can_payload& payload) {
        _current_state->handle_message(this, payload);
    }
};

} // namespace blk
} // namespace ucanopen

#endif
#endif
