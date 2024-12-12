#ifdef MCUDRV_C28X


#include "sdo_service.h"


namespace ucanopen {


unsigned char SdoService::cana_rsdo_dualcore_alloc[sizeof(can_payload)]
        __attribute__((section("shared_ucanopen_cana_rsdo_data"), retain));
unsigned char SdoService::canb_rsdo_dualcore_alloc[sizeof(can_payload)]
        __attribute__((section("shared_ucanopen_canb_rsdo_data"), retain));

unsigned char SdoService::cana_tsdo_dualcore_alloc[sizeof(can_payload)]
        __attribute__((section("shared_ucanopen_cana_tsdo_data"), retain));
unsigned char SdoService::canb_tsdo_dualcore_alloc[sizeof(can_payload)]
        __attribute__((section("shared_ucanopen_canb_tsdo_data"), retain));


const ODObjectKey SdoService::restore_default_parameter_key = {0x1011, 0x04};


SdoService::SdoService(impl::Server& server, const IpcFlags& ipc_flags)
        : _server(server),
          _rsdo_flag(ipc_flags.rsdo_received),
          _tsdo_flag(ipc_flags.tsdo_ready) {
    switch (_server._ipc_mode.underlying_value()) {
    case mcu::c28x::ipc::Mode::singlecore:
        _rsdo_data = new can_payload;
        _tsdo_data = new can_payload;
        break;
    case mcu::c28x::ipc::Mode::dualcore:
        switch (_server._can_peripheral.native_value()) {
        case mcu::c28x::can::Peripheral::cana:
            _rsdo_data = new(cana_rsdo_dualcore_alloc) can_payload;
            _tsdo_data = new(cana_tsdo_dualcore_alloc) can_payload;
            break;
        case mcu::c28x::can::Peripheral::canb:
            _rsdo_data = new(canb_rsdo_dualcore_alloc) can_payload;
            _tsdo_data = new(canb_tsdo_dualcore_alloc) can_payload;
            break;
        }
        break;
    }
}


void SdoService::recv() {
    assert(_server._ipc_role == mcu::c28x::ipc::Role::primary);

    if (_rsdo_flag.is_set() || _tsdo_flag.is_set()) {
        _server.on_sdo_overrun();
    } else {
        _server._can_module->recv(Cob::rsdo, _rsdo_data->data);
        _rsdo_flag.set();
    }
}

void SdoService::send() {
    assert(_server._ipc_role == mcu::c28x::ipc::Role::primary);

    if (!_tsdo_flag.is_set()) { return; }
    _server._can_module->send(Cob::tsdo, _tsdo_data->data, cob_data_len[Cob::tsdo]);
    _tsdo_flag.clear();
}


void SdoService::handle_received() {
    assert(_server._ipc_mode == mcu::c28x::ipc::Mode::singlecore || _server._ipc_role == mcu::c28x::ipc::Role::secondary);

    if (!_rsdo_flag.is_set()) { return; }   // no RSDO received

    ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(*_rsdo_data);
    _rsdo_flag.clear();
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

    switch (abort_code.native_value()) {
    case SdoAbortCode::no_error:
        to_payload<ExpeditedSdo>(*_tsdo_data, tsdo);
        break;
    default:
        AbortSdo abort_tsdo;
        abort_tsdo.index = rsdo.index;
        abort_tsdo.subindex = rsdo.subindex;
        abort_tsdo.error_code = abort_code.underlying_value();
        to_payload<AbortSdo>(*_tsdo_data, abort_tsdo);
        break;
    }

    _tsdo_flag.set();
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
        tsdo.data_empty_bytes = 4 - 2 * od_object_type_sizes[od_entry->object.data_type];
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
            arg_key.subindex &= 0xFF;
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
    assert(_server._ipc_mode == mcu::c28x::ipc::Mode::singlecore || _server._ipc_role == mcu::c28x::ipc::Role::secondary);

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
            memcpy(od_entry->object.ptr.first, &od_entry->object.default_value.value().u32, od_object_type_sizes[od_entry->object.data_type]);
        } else {
            memcpy(*od_entry->object.ptr.second, &od_entry->object.default_value.value().u32, od_object_type_sizes[od_entry->object.data_type]);
        }
        return SdoAbortCode::no_error;
    } else {
        return od_entry->object.write_func(od_entry->object.default_value.value());
    }
}


} // namespace ucanopen


#endif
