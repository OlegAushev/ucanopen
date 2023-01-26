#pragma once


#include <mcu_c28x/f2837xd/can/mcu_can.h>
#include <mcu_c28x/f2837xd/ipc/mcu_ipc.h>
#include <ucanopen_c28x/ucanopen_def.h>
#include <algorithm>


namespace ucanopen {

struct IpcFlags
{
	mcu::ipc::Flag rpdo1_received;
	mcu::ipc::Flag rpdo2_received;
	mcu::ipc::Flag rpdo3_received;
	mcu::ipc::Flag rpdo4_received;
	mcu::ipc::Flag rsdo_received;
	mcu::ipc::Flag tsdo_ready;
};


class ServerHeartbeatService;
class ServerTpdoService;
class ServerRpdoService;
class ServerSdoService;


namespace impl {

class Server
{
	friend class ucanopen::ServerHeartbeatService;
	friend class ucanopen::ServerTpdoService;
	friend class ucanopen::ServerRpdoService;
	friend class ucanopen::ServerSdoService;
protected:
	const mcu::ipc::Mode _ipc_mode;
	const mcu::ipc::Role _ipc_role;

	NodeId _node_id;
	mcu::can::Peripheral _can_peripheral;
	mcu::can::Module* _can_module;

	ODEntry* _dictionary;
	size_t _dictionary_size;

	NmtState _nmt_state;
private:
	emb::Array<mcu::can::MessageObject, cob_type_count> _message_objects;
public:
	Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary,
			NodeId node_id, mcu::can::Module* can_module,
			ODEntry* object_dictionary, size_t object_dictionary_size);

	Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary,
			NodeId node_id, mcu::can::Module* can_module);

	Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary,
			mcu::can::Peripheral can_peripheral, ODEntry* object_dictionary, size_t object_dictionary_size);

	NodeId node_id() const { return _node_id; }
	NmtState nmt_state() const { return _nmt_state; }
private:
	void _init_message_objects();
	void _init_object_dictionary();
};

} // namesppace impl

} // namespace ucanopen


