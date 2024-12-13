#ifdef MCUDRV_C28X

#include "rpdo_service.h"

namespace ucanopen {

RpdoService::RpdoService(impl::Server& server)
        : server_(server) {
    for (size_t i = 0; i < rpdo_msgs_.size(); ++i) {
        rpdo_msgs_[i].timeout = emb::chrono::milliseconds(0);
        rpdo_msgs_[i].timepoint = emb::chrono::milliseconds(0);
        rpdo_msgs_[i].unhandled = false;
        rpdo_msgs_[i].handler = NULL;
    }
}

void RpdoService::register_rpdo(CobRpdo rpdo,
                                emb::chrono::milliseconds timeout,
                                void (*handler)(const can_payload&),
                                can_id id) {
    const size_t idx = rpdo.underlying_value();
    rpdo_msgs_[idx].timeout = timeout;
    rpdo_msgs_[idx].timepoint = emb::chrono::steady_clock::now();
    rpdo_msgs_[idx].handler = handler;
    if (id != 0) {
        Cob cob = to_cob(rpdo);
        server_.message_objects_[cob.underlying_value()].frame_id = id;
        server_.can_module_.setup_message_object(
                server_.message_objects_[cob.underlying_value()]);
    }
}

void RpdoService::recv_frame(Cob cob) {
    if (cob != Cob::rpdo1 &&
        cob != Cob::rpdo2 &&
        cob != Cob::rpdo3 &&
        cob != Cob::rpdo4) {
        return;
    }

    const CobRpdo rpdo = CobRpdo((cob.underlying_value() - Cob::rpdo1) / 2);
    const size_t idx = rpdo.underlying_value();

    if (rpdo_msgs_[idx].unhandled) {
        server_.on_rpdo_overrun();
    } else {
        // there is no unprocessed RPDO of this type
        rpdo_msgs_[idx].timepoint = emb::chrono::steady_clock::now();
        server_.can_module_.recv(cob.underlying_value(),
                                 rpdo_msgs_[idx].payload.data);
        rpdo_msgs_[idx].unhandled = true;
    }
}

void RpdoService::handle_recv_frames() {
    for (size_t i = 0; i < rpdo_msgs_.size(); ++i) {
        if (rpdo_msgs_[i].unhandled && rpdo_msgs_[i].handler != NULL) {
            rpdo_msgs_[i].handler(rpdo_msgs_[i].payload);
            rpdo_msgs_[i].unhandled = false;
        }
    }
}

} // namespace ucanopen

#endif
