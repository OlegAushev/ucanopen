#if defined(MCUDRV_STM32) || defined(MCUDRV_APM32)
#if defined(STM32F4xx) || defined(APM32F4xx)


#include "sdo_service.h"


namespace ucanopen {


const ODObjectKey SdoService::restore_default_parameter_key = {0x1011, 0x04};


SdoService::SdoService(impl::Server& server)
        : _server(server) {
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
        .SlaveStartFilterBank = {}
    };
#elif defined(MCUDRV_APM32)
    CAN_FilterConfig_T rsdo_filter = {
        .filterNumber{},
        .filterIdHigh = uint16_t(calculate_cob_id(Cob::rsdo, _server.node_id()) << 5),
        .filterIdLow = 0,
        .filterMaskIdHigh = 0x7FF << 5,
        .filterMaskIdLow = 0,
        .filterActivation{},
        .filterFIFO = CAN_FILTER_FIFO_0,
        .filterMode = CAN_FILTER_MODE_IDMASK,
        .filterScale = CAN_FILTER_SCALE_32BIT
    };
#endif
    _rsdo_rxattr = _server._can_module.register_rxmessage(rsdo_filter);
    _tsdo_id = calculate_cob_id(Cob::tsdo, _server.node_id());
}


std::vector<mcu::can::RxMessageAttribute> SdoService::get_rx_attr() const {
    return {_rsdo_rxattr};
}


FrameRecvStatus SdoService::recv_frame(const mcu::can::RxMessageAttribute& attr, const can_frame& frame) {
    if (attr != _rsdo_rxattr) {
        return FrameRecvStatus::attr_mismatch;
    }

    if (_rsdo_queue.full()) {
        return FrameRecvStatus::overrun;
    }

    _rsdo_queue.push(frame.payload);
    return FrameRecvStatus::success;
}


void SdoService::handle_recv_frames() {
    while (!_rsdo_queue.empty()) {
        can_payload rsdo_payload = _rsdo_queue.front();
        ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(rsdo_payload);
        _rsdo_queue.pop();

        if (rsdo.cs == sdo_cs_codes::abort) {
            continue;
        }

        ExpeditedSdo tsdo;
        SdoAbortCode abort_code = SdoAbortCode::general_error;
        ODEntry* dictionary_end = _server._dictionary + _server._dictionary_size;
        ODObjectKey key = {static_cast<uint16_t>(rsdo.index), static_cast<uint8_t>(rsdo.subindex)};

        const ODEntry* od_entry = emb::binary_find(_server._dictionary, dictionary_end, key);

        if (od_entry == dictionary_end) {
            abort_code = SdoAbortCode::object_not_found;
        }
        else if (rsdo.cs == sdo_cs_codes::client_init_read) {
            abort_code = _read_expedited(od_entry, tsdo, rsdo);
        } else if (rsdo.cs == sdo_cs_codes::client_init_write) {
            abort_code = _write_expedited(od_entry, tsdo, rsdo);
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

        if (!_tsdo_queue.full()) {
            _tsdo_queue.push(tsdo_payload);
        }
    }
}


SdoAbortCode SdoService::_read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo) {
    if (!od_entry->object.has_read_permission()) {
        return SdoAbortCode::read_access_wo;
    }

    SdoAbortCode abort_code;
    if (od_entry->object.has_direct_access()) {
        if (od_entry->object.ptr.first) {
            memcpy(&tsdo.data.u32, od_entry->object.ptr.first, od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(&tsdo.data.u32, *od_entry->object.ptr.second, od_object_type_sizes[od_entry->object.data_type]);
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
        tsdo.data_empty_bytes = (4 - od_object_type_sizes[od_entry->object.data_type]) & 0x3;
    }
    return abort_code;
}


SdoAbortCode SdoService::_write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo) {
    if (!od_entry->object.has_write_permission()) {
        return SdoAbortCode::write_access_ro;
    }

    SdoAbortCode abort_code;
    if (od_entry->object.has_direct_access()) {
        if (od_entry->object.ptr.first) {
            memcpy(od_entry->object.ptr.first, &rsdo.data.u32, od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(*od_entry->object.ptr.second, &rsdo.data.u32, od_object_type_sizes[od_entry->object.data_type]);
        }
        abort_code = SdoAbortCode::no_error;
    } else {
        abort_code = od_entry->object.write_func(rsdo.data);
    }

    if (abort_code == SdoAbortCode::data_store_error) {
        if (od_entry->key == restore_default_parameter_key) {
            ODObjectKey arg_key = {};
            memcpy(&arg_key, &rsdo.data.u32, sizeof(arg_key));
            abort_code = _restore_default_parameter(arg_key);
        }
    }

    if (abort_code == SdoAbortCode::no_error) {
        tsdo.index = rsdo.index;
        tsdo.subindex = rsdo.subindex;
        tsdo.cs = sdo_cs_codes::server_init_write;
    }
    return abort_code;
}


SdoAbortCode SdoService::_restore_default_parameter(ODObjectKey key) {
    ODEntry* dictionary_end = _server._dictionary + _server._dictionary_size;
    const ODEntry* od_entry = emb::binary_find(_server._dictionary, dictionary_end, key);

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
            memcpy(od_entry->object.ptr.first, &od_entry->object.default_value->u32, od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(*od_entry->object.ptr.second, &od_entry->object.default_value->u32, od_object_type_sizes[od_entry->object.data_type]);
        }
        return SdoAbortCode::no_error;
    } else {
        return od_entry->object.write_func(*od_entry->object.default_value);
    }
}


void SdoService::send() {
    while (!_tsdo_queue.empty()) {
        can_payload payload = _tsdo_queue.front();
        _server._can_module.put_frame({_tsdo_id, _tsdo_len, payload});
        _tsdo_queue.pop();
    }
}


} // namespace ucanopen


#endif
#endif
