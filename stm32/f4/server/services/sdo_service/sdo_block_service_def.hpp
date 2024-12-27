#pragma once

#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/ucanopen_def.hpp>

namespace ucanopen {

namespace blk_write {

struct ClientRequestSdo {
    uint32_t client_subcommand : 1;
    uint32_t size_indicator : 1;
    uint32_t crc_support : 1;
    uint32_t _reserved : 2;
    uint32_t ccs : 3;
    uint32_t index : 16;
    uint32_t subindex : 8;
    uint32_t data_block_size;

    bool valid() const {
        return client_subcommand == 0 &&
               ccs == sdo_cs_codes::client_block_write;
    }
};

struct ServerResponseSdo {
    uint32_t server_subcommand : 2;
    uint32_t crc_support : 1;
    uint32_t _reserved1 : 2;
    uint32_t scs : 3;
    uint32_t index : 16;
    uint32_t subindex : 8;
    uint32_t block_size : 8;
    uint32_t _reserved2 : 24;

    bool valid() const {
        return server_subcommand == 0 &&
               scs == sdo_cs_codes::server_block_write && block_size >= 1 &&
               block_size <= 128;
    }
};

struct ClientSegmentSdo {
    uint8_t sequence_counter : 7;
    uint8_t last_segment : 1;
    uint8_t data[7];

    bool valid() const {
        return sequence_counter >= 1 && sequence_counter <= 127;
    }
};

struct ServerSegmentResponseSdo {
    uint32_t server_subcommand : 2;
    uint32_t _reserved1 : 3;
    uint32_t scs : 3;
    uint32_t segments_acknowledged : 8;
    uint32_t next_block_size : 8;
    uint32_t _reserved2 : 8;
    uint32_t _reserved3;

    bool valid() const {
        return server_subcommand == 2 &&
               scs == sdo_cs_codes::server_block_write &&
               next_block_size >= 1 && next_block_size <= 127;
    }
};

struct ClientEndSdo {
    uint32_t client_subcommand : 1;
    uint32_t _reserved1 : 1;
    uint32_t n : 3;
    uint32_t ccs : 3;
    uint32_t crc : 16;
    uint32_t _reserved2 : 8;
    uint32_t _reserved3;

    bool valid() const {
        return client_subcommand == 1 &&
               ccs == sdo_cs_codes::client_block_write;
    }
};

struct ServerEndResponseSdo {
    uint8_t server_subcommand : 2;
    uint8_t _reserved1 : 3;
    uint8_t scs : 3;
    uint8_t _reserved2[7];

    bool valid() const {
        return server_subcommand == 1 &&
               scs == sdo_cs_codes::server_block_write;
    }
};

} // namespace blk_write

namespace blk_read {} // namespace blk_read

} // namespace ucanopen

#endif
#endif
