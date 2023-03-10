#include "server.h"


namespace ucanopen {

Server::Server(mcu::ipc::traits::singlecore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
		NodeId nodeId, mcu::can::Module* can_module,
		ODEntry* object_dictionary, size_t object_dictionary_size)
	: impl::Server(mcu::ipc::traits::singlecore(), mcu::ipc::traits::primary(),
			nodeId, can_module, object_dictionary, object_dictionary_size)
	, emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, can_module->peripheral().underlying_value())
{
	heartbeat_service = new HeartbeatService(this, emb::chrono::milliseconds(1000));
	tpdo_service = new TpdoService(this);
	rpdo_service = new RpdoService(this, ipc_flags);
	sdo_service = new SdoService(this, ipc_flags);

	switch (can_module->peripheral().native_value())
	{
		case mcu::can::Peripheral::cana:
			this->_can_module->register_interrupt_callback(on_frame_received<mcu::can::Peripheral::cana>);
			break;
		case mcu::can::Peripheral::canb:
			this->_can_module->register_interrupt_callback(on_frame_received<mcu::can::Peripheral::canb>);
			break;
	}

	this->_nmt_state = NmtState::pre_operational;
}


Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::primary, const IpcFlags& ipc_flags,
		NodeId nodeId, mcu::can::Module* can_module)
	: impl::Server(mcu::ipc::traits::dualcore(), mcu::ipc::traits::primary(), nodeId, can_module)
	, emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, can_module->peripheral().underlying_value())
{
	heartbeat_service = new HeartbeatService(this, emb::chrono::milliseconds(1000));
	tpdo_service = new TpdoService(this);
	rpdo_service = new RpdoService(this, ipc_flags);
	sdo_service = new SdoService(this, ipc_flags);

	switch (can_module->peripheral().native_value())
	{
		case mcu::can::Peripheral::cana:
			this->_can_module->register_interrupt_callback(on_frame_received<mcu::can::Peripheral::cana>);
			break;
		case mcu::can::Peripheral::canb:
			this->_can_module->register_interrupt_callback(on_frame_received<mcu::can::Peripheral::canb>);
			break;
	}

	this->_nmt_state = NmtState::pre_operational;
}


Server::Server(mcu::ipc::traits::dualcore, mcu::ipc::traits::secondary, const IpcFlags& ipc_flags,
		mcu::can::Peripheral can_peripheral, ODEntry* object_dictionary, size_t object_dictionary_size)
	: impl::Server(mcu::ipc::traits::dualcore(), mcu::ipc::traits::secondary(), can_peripheral, object_dictionary, object_dictionary_size)
	, emb::c28x::interrupt_invoker_array<Server, mcu::can::peripheral_count>(this, can_peripheral.underlying_value())
{
	rpdo_service = new RpdoService(this, ipc_flags);
	sdo_service = new SdoService(this, ipc_flags);
}

} // namespace ucanopen

