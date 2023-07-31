#pragma once


#ifdef STM32F4xx


#include "../node/node.h"


namespace ucanopen {

namespace tests {

class Node : public ucanopen::Node {
public:
    Node(ucanopen::Server& server, uint32_t id, uint32_t rx_location)
            : ucanopen::Node(server) {
        FDCAN_TxHeaderTypeDef tx1_header = {
            .Identifier = 0x101 + 0x10*id,
            .IdType = FDCAN_STANDARD_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8,
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };
        register_tx_message(tx1_header, std::chrono::milliseconds(200), _create_tx1);
        
        FDCAN_TxHeaderTypeDef tx2_header = {
            .Identifier = 0x102 + 0x10*id,
            .IdType = FDCAN_STANDARD_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8,
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };
        register_tx_message(tx2_header, std::chrono::milliseconds(200), _create_tx2);

        FDCAN_TxHeaderTypeDef tx3_header = {
            .Identifier = 0x103 + 0x10*id,
            .IdType = FDCAN_STANDARD_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8,
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };
        register_tx_message(tx3_header, std::chrono::milliseconds(200), _create_tx3);
        
        FDCAN_TxHeaderTypeDef tx4_header = {
            .Identifier = 0x104 + 0x10*id,
            .IdType = FDCAN_STANDARD_ID,
            .TxFrameType = FDCAN_DATA_FRAME,
            .DataLength = FDCAN_DLC_BYTES_8,
            .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
            .BitRateSwitch = FDCAN_BRS_OFF,
            .FDFormat = FDCAN_CLASSIC_CAN,
            .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
            .MessageMarker = 0
        };
        register_tx_message(tx4_header, std::chrono::milliseconds(200), _create_tx4);

        FDCAN_FilterTypeDef rx1_filter = {
            .IdType = FDCAN_STANDARD_ID,
            .FilterIndex = 0,
            .FilterType = FDCAN_FILTER_MASK,
            .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
            .FilterID1 = 0x105 + 0x10*id,
            .FilterID2 = 0x7FF,
            .RxBufferIndex = 0,
            .IsCalibrationMsg = 0
        };
        register_rx_message(rx1_filter, std::chrono::milliseconds(1000), _handle_rx1);

        FDCAN_FilterTypeDef rx2_filter = {
            .IdType = FDCAN_STANDARD_ID,
            .FilterIndex = 0,
            .FilterType = FDCAN_FILTER_MASK,
            .FilterConfig = rx_location,
            .FilterID1 = 0x106 + 0x10*id,
            .FilterID2 = 0x7FF,
            .RxBufferIndex = 0,
            .IsCalibrationMsg = 0
        };
        register_rx_message(rx2_filter, std::chrono::milliseconds(1000), _handle_rx2);
    };
private:
    static inline uint64_t value1 = 0;
    static inline uint64_t value2 = 0;

    static can_payload _create_tx1() {
        static uint64_t counter = 0;
        return to_payload<uint64_t>(counter++);
    }

    static can_payload _create_tx2() {
        static uint64_t counter = 0;
        return to_payload<uint64_t>(counter++);
    }

    static can_payload _create_tx3() {
        return to_payload<uint64_t>(value1);
    }

    static can_payload _create_tx4() {
        return to_payload<uint64_t>(value2);
    }

    static void _handle_rx1(const can_payload& payload) {
        value1 = from_payload<uint64_t>(payload);
    }

    static void _handle_rx2(const can_payload& payload) {
        value2 = from_payload<uint64_t>(payload);
    }
};

} // namespace ucanopen

} // namespace tests

#endif
