#ifdef MCUDRV_C28X

#include "sdo_service.h"

namespace ucanopen {

const ODObjectKey SdoService::restore_default_parameter_key = {0x1011, 0x04};

SdoService::SdoService(impl::Server& server) : server_(server) {}


void SdoService::recv_frame() {
    if (rsdo_queue_.full()) {
        server_.on_sdo_overrun();
    } else {
        can_payload payload;
        server_.can_module_.recv(Cob::rsdo, payload.data);
        rsdo_queue_.push(payload);
    }
}

void SdoService::send() {
    while (!tsdo_queue_.empty()) {
        can_payload payload = tsdo_queue_.front();
        server_.can_module_.send(Cob::tsdo,
                                 payload.data,
                                 cob_data_len[Cob::tsdo]);
        tsdo_queue_.pop();
    }
}


void SdoService::handle_recv_frames() {
    while (!rsdo_queue_.empty()) {
        can_payload rsdo_payload = rsdo_queue_.front();
        ExpeditedSdo rsdo = from_payload<ExpeditedSdo>(rsdo_payload);
        rsdo_queue_.pop();

        if (rsdo.cs == sdo_cs_codes::abort) {
            return;
        }

        ExpeditedSdo tsdo;
        SdoAbortCode abort_code = SdoAbortCode::general_error;
        ODEntry* dictionary_end = server_.dictionary_ +
                                  server_.dictionary_size_;
        ODObjectKey key = {rsdo.index, rsdo.subindex};

        const ODEntry* od_entry = emb::binary_find(server_.dictionary_,
                                                   dictionary_end,
                                                   key);

        if (od_entry == dictionary_end) {
            abort_code = SdoAbortCode::object_not_found;
        }
        else if (rsdo.cs == sdo_cs_codes::client_init_read) {
            abort_code = read_expedited(od_entry, tsdo, rsdo);
        } else if (rsdo.cs == sdo_cs_codes::client_init_write) {
            abort_code = write_expedited(od_entry, tsdo, rsdo);
        } else {
            abort_code = SdoAbortCode::invalid_cs;
        }

        can_payload tsdo_payload;
        switch (abort_code.native_value()) {
        case SdoAbortCode::no_error:
            tsdo_payload = to_payload<ExpeditedSdo>(tsdo);
            break;
        default:
            AbortSdo abort_tsdo;
            abort_tsdo.index = rsdo.index;
            abort_tsdo.subindex = rsdo.subindex;
            abort_tsdo.error_code = abort_code.underlying_value();
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

SdoAbortCode SdoService::write_expedited(const ODEntry* od_entry,
                                         ExpeditedSdo& tsdo,
                                         const ExpeditedSdo& rsdo) {
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
    const ODEntry* od_entry = emb::binary_find(server_.dictionary_,
                                               dictionary_end,
                                               key);

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
