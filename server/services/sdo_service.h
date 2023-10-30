#pragma once


#include "../impl/impl_server.h"
#include <emblib/algorithm.h>
#include <new>


namespace ucanopen {

class SdoService {
private:
    impl::Server& _server;

    mcu::ipc::Flag _rsdo_flag;
    mcu::ipc::Flag _tsdo_flag;
    can_payload* _rsdo_data;
    can_payload* _tsdo_data;
    static unsigned char cana_rsdo_dualcore_alloc[sizeof(can_payload)];
    static unsigned char canb_rsdo_dualcore_alloc[sizeof(can_payload)];
    static unsigned char cana_tsdo_dualcore_alloc[sizeof(can_payload)];
    static unsigned char canb_tsdo_dualcore_alloc[sizeof(can_payload)];
public:
    SdoService(impl::Server& server, const IpcFlags& ipc_flags);
    void handle_received();

    void recv() {
        assert(_server._ipc_role == mcu::ipc::Role::primary);

        if (_rsdo_flag.local.is_set() || _tsdo_flag.is_set()) {
            _server.on_sdo_overrun();
        } else {
            _server._can_module->recv(Cob::rsdo, _rsdo_data->data);
            _rsdo_flag.local.set();
        }
    }

    void send() {
        assert(_server._ipc_role == mcu::ipc::Role::primary);

        if (!_tsdo_flag.is_set()) { return; }
        _server._can_module->send(Cob::tsdo, _tsdo_data->data, cob_data_len[Cob::tsdo]);
        _tsdo_flag.reset();
    }

private:
    SdoAbortCode _read_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _write_expedited(const ODEntry* od_entry, ExpeditedSdo& tsdo, const ExpeditedSdo& rsdo);
    SdoAbortCode _restore_default_parameter(ODObjectKey key);

    static const ODObjectKey restore_default_parameter_key;
};

} // namespace ucanopen

