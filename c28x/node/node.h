#pragma once

#ifdef MCUDRV_C28X

#include "../ucanopen_def.h"
#include <emblib/chrono.h>
#include <emblib/static_vector.h>

namespace ucanopen {

class Server;

class Node {
private:
    Server& server_;

    struct RxMessage {
        uint32_t obj_id;
        emb::chrono::milliseconds timeout;
        emb::chrono::milliseconds timepoint;
        bool unhandled;
        can_payload payload;
        uint16_t len;
        void (*handler)(const can_payload&);
    };
    emb::static_vector<RxMessage, 4> rx_msgs_;

    struct TxMessage {
        uint32_t obj_id;
        emb::chrono::milliseconds period;
        emb::chrono::milliseconds timepoint;
        uint16_t len;
        can_payload (*creator)();
    };
    emb::static_vector<TxMessage, 4> tx_msgs_;
public:
    Node(Server& server);
    void register_rx_message(can_id id,
                             uint16_t len,
                             emb::chrono::milliseconds timeout,
                             void (*handler)(const can_payload&));
    void register_tx_message(can_id id,
                             uint16_t len,
                             emb::chrono::milliseconds period,
                             can_payload (*creator)());
    void recv_frame(uint32_t obj_id);
    void send();
};

} // namespace ucanopen

#endif
