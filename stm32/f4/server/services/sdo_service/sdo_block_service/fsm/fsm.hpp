#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_block_service/sdo_block_service_def.hpp>

#include <emblib/fsm.hpp>

namespace ucanopen {

class SdoBlockService;

namespace blk_fsm {

constexpr size_t state_count = 3;
enum class State : unsigned int {
    idle,
    download,
    download_end,
};

class AbstractState : public emb::fsm::abstract_state<SdoBlockService, State> {
protected:
    AbstractState(State id)
            : emb::fsm::abstract_state<SdoBlockService, State>(id) {}
public:
    static AbstractState* create(State state);
    static void destroy(State state, AbstractState* stateobj);
    virtual ~AbstractState() {}

    virtual void handle(SdoBlockService* _service,
                        const can_payload& payload) = 0;
};

class IdleState final : public AbstractState {
protected:
    virtual void initiate(SdoBlockService* _service) override {}
    virtual void finalize(SdoBlockService* _service) override {}
public:
    IdleState() : AbstractState(State::idle) {}
    virtual void handle(SdoBlockService* _service,
                        const can_payload& payload) override;
};

class DownloadState final : public AbstractState {
protected:
    virtual void initiate(SdoBlockService* _service) override {}
    virtual void finalize(SdoBlockService* _service) override {}
public:
    DownloadState() : AbstractState(State::download) {}
    virtual void handle(SdoBlockService* _service,
                        const can_payload& payload) override;
};

class DownloadEndState final : public AbstractState {
protected:
    virtual void initiate(SdoBlockService* _service) override {}
    virtual void finalize(SdoBlockService* _service) override {}
public:
    DownloadEndState() : AbstractState(State::download_end) {}
    virtual void handle(SdoBlockService* _service,
                        const can_payload& payload) override;
};

} // namespace blk_fsm
} // namespace ucanopen

#endif
#endif
