#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)

#include <ucanopen/stm32/f4/server/services/sdo_service/sdo_service.hpp>

namespace ucanopen {

const ODObjectKey SdoService::restore_default_parameter_key = {0x1011, 0x04};

SdoService::SdoService(impl::Server& server) : server_(server) {
#if defined(MCUDRV_STM32)
    CAN_FilterTypeDef rsdo_filter = {
        .FilterIdHigh = calculate_cob_id(Cob::rsdo, _server.node_id()) << 5,
        .FilterIdLow = 0,
        .FilterMaskIdHigh = 0x7FF << 5,
        .FilterMaskIdLow = 0,
        .FilterFIFOAssignment = CAN_RX_FIFO0,
        .FilterBank = {},
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_32BIT,
        .FilterActivation = {},
        .SlaveStartFilterBank = {}};
#elif defined(MCUDRV_APM32)
    CAN_FilterConfig_T rsdo_filter = {
        .filterNumber{},
        .filterIdHigh =
                uint16_t(calculate_cob_id(Cob::rsdo, server_.node_id()) << 5),
        .filterIdLow = 0,
        .filterMaskIdHigh = 0x7FF << 5,
        .filterMaskIdLow = 0,
        .filterActivation{},
        .filterFIFO = CAN_FILTER_FIFO_0,
        .filterMode = CAN_FILTER_MODE_IDMASK,
        .filterScale = CAN_FILTER_SCALE_32BIT};
#endif
    rsdo_rxattr_ = server_.can_module_.register_rxmessage(rsdo_filter);
    tsdo_id_ = calculate_cob_id(Cob::tsdo, server_.node_id());
}

std::vector<ucan::RxMessageAttribute> SdoService::get_rx_attr() const {
    return {rsdo_rxattr_};
}

void SdoService::recv(const ucan::RxMessageAttribute& attr,
                      const can_frame& frame) {
    if (attr != rsdo_rxattr_) {
        return;
    }

    if (rsdo_queue_.full()) {
        server_.on_sdo_overrun();
        return;
    }

    rsdo_queue_.push(frame.payload);
}

void SdoService::handle() {
    while (!rsdo_queue_.empty()) {
        can_payload rsdo_payload = rsdo_queue_.front();
        ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(rsdo_payload);
        rsdo_queue_.pop();

        if (rsdo.cs == sdo_cs_codes::abort) {
            continue;
        }

        ExpeditedSdo tsdo;
        SdoAbortCode abort_code = SdoAbortCode::general_error;
        ODEntry* dictionary_end =
                server_.dictionary_ + server_.dictionary_size_;
        ODObjectKey key = {static_cast<uint16_t>(rsdo.index),
                           static_cast<uint8_t>(rsdo.subindex)};

        const ODEntry* od_entry =
                emb::binary_find(server_.dictionary_, dictionary_end, key);

        if (od_entry == dictionary_end) {
            abort_code = SdoAbortCode::object_not_found;
        } else if (rsdo.cs == sdo_cs_codes::client_init_read) {
            abort_code = read_expedited(od_entry, tsdo, rsdo);
        } else if (rsdo.cs == sdo_cs_codes::client_init_write) {
            abort_code = write_expedited(od_entry, tsdo, rsdo);
        } else {
            abort_code = SdoAbortCode::invalid_cs;
        }

        can_payload tsdo_payload;
        switch (abort_code) {
        case SdoAbortCode::no_error:
            tsdo_payload = to_payload<ExpeditedSdo>(tsdo);
            break;
        default:
            AbortSdo abort_tsdo(rsdo.index, rsdo.subindex, abort_code);
            tsdo_payload = to_payload<AbortSdo>(abort_tsdo);
            break;
        }

        if (!tsdo_queue_.full()) {
            tsdo_queue_.push(tsdo_payload);
        }
    }
}

SdoAbortCode SdoService::read_expedited(const ODEntry* od_entry,
                                        ExpeditedSdo& tsdo,
                                        const ExpeditedSdo& rsdo) {
    if (!od_entry->object.has_read_permission()) {
        return SdoAbortCode::read_access_wo;
    }

    SdoAbortCode abort_code;
    if (od_entry->object.has_direct_access()) {
        if (od_entry->object.ptr.first) {
            memcpy(&tsdo.data.u32,
                   od_entry->object.ptr.first,
                   od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(&tsdo.data.u32,
                   *od_entry->object.ptr.second,
                   od_object_type_sizes[od_entry->object.data_type]);
        }
        abort_code = SdoAbortCode::no_error;
    } else {
        abort_code = od_entry->object.read_func(tsdo.data);
    }

    if (abort_code == SdoAbortCode::no_error) {
        tsdo.index = rsdo.index;
        tsdo.subindex = rsdo.subindex;
        tsdo.cs = sdo_cs_codes::server_init_read;
        tsdo.expedited_transfer = 1;
        tsdo.data_size_indicated = 1;
        tsdo.data_empty_bytes =
                (4 - od_object_type_sizes[od_entry->object.data_type]) & 0x3;
    }
    return abort_code;
}

SdoAbortCode SdoService::write_expedited(const ODEntry* od_entry,
                                         ExpeditedSdo& tsdo,
                                         const ExpeditedSdo& rsdo) {
    if (!od_entry->object.has_write_permission()) {
        return SdoAbortCode::write_access_ro;
    }

    SdoAbortCode abort_code;
    if (od_entry->object.has_direct_access()) {
        if (od_entry->object.ptr.first) {
            memcpy(od_entry->object.ptr.first,
                   &rsdo.data.u32,
                   od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(*od_entry->object.ptr.second,
                   &rsdo.data.u32,
                   od_object_type_sizes[od_entry->object.data_type]);
        }
        abort_code = SdoAbortCode::no_error;
    } else {
        abort_code = od_entry->object.write_func(rsdo.data);
    }

    if (abort_code == SdoAbortCode::data_store_error) {
        if (od_entry->key == restore_default_parameter_key) {
            ODObjectKey arg_key = {};
            memcpy(&arg_key, &rsdo.data.u32, sizeof(arg_key));
            abort_code = restore_default_parameter(arg_key);
        }
    }

    if (abort_code == SdoAbortCode::no_error) {
        tsdo.index = rsdo.index;
        tsdo.subindex = rsdo.subindex;
        tsdo.cs = sdo_cs_codes::server_init_write;
    }
    return abort_code;
}

SdoAbortCode SdoService::restore_default_parameter(ODObjectKey key) {
    ODEntry* dictionary_end = server_.dictionary_ + server_.dictionary_size_;
    const ODEntry* od_entry =
            emb::binary_find(server_.dictionary_, dictionary_end, key);

    if (od_entry == dictionary_end) {
        return SdoAbortCode::object_not_found;
    }

    if (!od_entry->object.default_value.has_value()) {
        return SdoAbortCode::data_store_error;
    }

    if (!od_entry->object.has_write_permission()) {
        return SdoAbortCode::write_access_ro;
    }

    if (od_entry->object.has_direct_access()) {
        if (od_entry->object.ptr.first) {
            memcpy(od_entry->object.ptr.first,
                   &od_entry->object.default_value->u32,
                   od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(*od_entry->object.ptr.second,
                   &od_entry->object.default_value->u32,
                   od_object_type_sizes[od_entry->object.data_type]);
        }
        return SdoAbortCode::no_error;
    } else {
        return od_entry->object.write_func(*od_entry->object.default_value);
    }
}

void SdoService::send() {
    while (!tsdo_queue_.empty()) {
        can_payload payload = tsdo_queue_.front();
        server_.can_module_.put_frame({tsdo_id_, tsdo_len_, payload});
        tsdo_queue_.pop();
    }
}

} // namespace ucanopen

#endif
#endif
