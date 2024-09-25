#pragma once


#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include <ucanopen/stm32/f4/server/impl/impl_server.h>


namespace ucanopen {


// struct ClientBlockWriteRequestSdo {
//     uint32_t data_size_indicated : 1;
//     uint32_t expedited_transfer : 1;
//     uint32_t data_empty_bytes : 2;
//     uint32_t _reserved : 1;
//     uint32_t cs : 3;
//     uint32_t index : 16;
//     uint32_t subindex : 8;
//     ExpeditedSdoData data;
// };


// struct ClientBlockWriteResponseSdo {
//     uint32_t data_size_indicated : 1;
//     uint32_t expedited_transfer : 1;
//     uint32_t data_empty_bytes : 2;
//     uint32_t _reserved : 1;
//     uint32_t cs : 3;
//     uint32_t index : 16;
//     uint32_t subindex : 8;
//     ExpeditedSdoData data;
// };









} // namespace ucanopen


#endif
#endif
