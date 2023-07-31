#ifdef STM32H7xx


#include "sdo_service.h"


namespace ucanopen {

SdoService::SdoService(impl::Server& server)
        : _server(server) {
    FDCAN_FilterTypeDef rsdo_filter = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_MASK,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = calculate_cob_id(CobType::rsdo, _server.node_id()),
        .FilterID2 = 0x7FF,
        .RxBufferIndex = 0,
        .IsCalibrationMsg = 0
    };
    _rsdo.attr = _server._can_module.register_message(rsdo_filter);
    _rsdo.is_unhandled = false;

    _tsdo.header = {
        .Identifier = calculate_cob_id(CobType::tsdo, _server.node_id()),
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = FDCAN_DLC_BYTES_8,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker = 0
    };    
    _tsdo.not_sent = false;
}


std::vector<mcu::can::MessageAttribute> SdoService::get_rx_attr() const {
    return {_rsdo.attr};
}


FrameRecvStatus SdoService::recv_frame(const mcu::can::MessageAttribute& attr, const can_frame& frame) {
    if (attr != _rsdo.attr) {
        return FrameRecvStatus::attr_mismatch;
    }

    if (_rsdo.is_unhandled) {
        return FrameRecvStatus::overrun;
    }

    _rsdo.frame = frame;
    _rsdo.is_unhandled = true;
    return FrameRecvStatus::success;
}


void SdoService::handle_recv_frames() {
    if (!_rsdo.is_unhandled) { return; }

    ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(_rsdo.frame.payload);
    _rsdo.is_unhandled = false;

    if (rsdo.cs == sdo_cs_codes::abort) {
        return;
    }

    ExpeditedSdo tsdo;
    SdoAbortCode abort_code = SdoAbortCode::general_error;
    ODEntry* dictionary_end = _server._dictionary + _server._dictionary_size;
    ODObjectKey key = {rsdo.index, rsdo.subindex};

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

    switch (abort_code) {
    case SdoAbortCode::no_error:
        _tsdo.payload = to_payload<ExpeditedSdo>(tsdo);
        break;
    default:
        AbortSdo abort_tsdo;
        abort_tsdo.index = rsdo.index;
        abort_tsdo.subindex = rsdo.subindex;
        abort_tsdo.error_code = std::to_underlying(abort_code);
        _tsdo.payload = to_payload<AbortSdo>(abort_tsdo);
        break;
    }

    _tsdo.not_sent = true;
}


SdoAbortCode SdoService::_read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo) {
    if (!od_entry->object.has_read_permission()) {
        return SdoAbortCode::read_access_wo;
    }

    SdoAbortCode abort_code;
    if (od_entry->object.has_direct_access()) {
        memcpy(&tsdo.data.u32, od_entry->object.ptr, od_object_type_sizes[od_entry->object.type]);
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
        tsdo.data_empty_bytes = 4 - 2 * od_object_type_sizes[od_entry->object.type];
    }
    return abort_code;
}


SdoAbortCode SdoService::_write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo) {
    if (!od_entry->object.has_write_permission()) {
        return SdoAbortCode::write_access_ro;
    }

    SdoAbortCode abort_code;
    if (od_entry->object.has_direct_access()) {
        memcpy(od_entry->object.ptr, &rsdo.data.u32, od_object_type_sizes[od_entry->object.type]);
        abort_code = SdoAbortCode::no_error;
    } else {
        abort_code = od_entry->object.write_func(rsdo.data);
    }

    if (abort_code == SdoAbortCode::no_error) {
        tsdo.index = rsdo.index;
        tsdo.subindex = rsdo.subindex;
        tsdo.cs = sdo_cs_codes::server_init_write;
    }
    return abort_code;
}


void SdoService::send() {
    if (!_tsdo.not_sent) { return; }
    _server._can_module.send(_tsdo.header, _tsdo.payload);
    _tsdo.not_sent = false;
}

} // namespace ucanopen

#endif
