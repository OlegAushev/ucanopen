#include <c28x_ucanopen/tests/ucanopen_tests.h>


namespace ucanopen {

namespace tests {

Object* Server::_object;
unsigned char object_alloc[sizeof(Object)] __attribute__((section("shared_ucanopen_test_object")));


namespace od {

inline ODAccessStatus get_device_name(ExpeditedSdoData& retval)
{
	char name[4] = {0};
	strncpy(name, sysinfo::device_name_short, 4);
	uint32_t name_raw = 0;
	emb::c28x::from_bytes<uint32_t>(name_raw, reinterpret_cast<uint16_t*>(name));
	retval.u32 = name_raw;
	return ODAccessStatus::success;
}


inline ODAccessStatus get_firmware_version(ExpeditedSdoData& retval)
{
	char name[4] = {0};
	strncpy(name, GIT_HASH, 4);
	uint32_t name_raw = 0;
	emb::c28x::from_bytes<uint32_t>(name_raw, reinterpret_cast<uint16_t*>(name));
	retval.u32 = name_raw;
	return ODAccessStatus::success;
}


inline ODAccessStatus get_build_configuration(ExpeditedSdoData& retval)
{
	char name[4] = {0};
	strncpy(name, sysinfo::build_configuration_short, 4);
	uint32_t name_raw = 0;
	emb::c28x::from_bytes<uint32_t>(name_raw, reinterpret_cast<uint16_t*>(name));
	retval.u32 = name_raw;
	return ODAccessStatus::success;
}


inline ODAccessStatus reset_device(ExpeditedSdoData& retval)
{
	syslog::add_message(sys::Message::device_software_resetting);
	mcu::chrono::system_clock::register_delayed_task(mcu::reset_device, 2000);
	return ODAccessStatus::success;
}


inline ODAccessStatus reset_errors(ExpeditedSdoData& retval)
{
	syslog::reset_errors_warnings();
	return ODAccessStatus::success;
}


inline ODAccessStatus get_syslog_message(ExpeditedSdoData& retval)
{
	retval.u32 = syslog::read_message().underlying_value();
	syslog::pop_message();
	return ODAccessStatus::success;
}

inline ODAccessStatus get_uptime(ExpeditedSdoData& retval)
{
	retval.f32 = mcu::chrono::system_clock::now() / 1000.f;
	return ODAccessStatus::success;
}

} // namespace od


ODEntry object_dictionary[] = {
{{0x1008, 0x00}, {"system", "info", "device_name", "", OD_STRING_4CHARS, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_device_name, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x00}, {"system", "info", "firmware_version", "", OD_STRING_4CHARS, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_firmware_version, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x01}, {"system", "info", "build_configuration", "", OD_STRING_4CHARS, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_build_configuration, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2000, 0x00}, {"system", "device", "reset_device", "", OD_EXEC, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::reset_device, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x2000, 0x01}, {"system", "syslog", "reset_errors", "", OD_EXEC, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::reset_errors, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x5000, 0x00}, {"watch", "watch", "uptime", "s", OD_FLOAT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_uptime, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x01}, {"watch", "watch", "syslog_message", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_syslog_message, OD_NO_INDIRECT_WRITE_ACCESS}},

};


const size_t object_dictionary_size = sizeof(object_dictionary) / sizeof(object_dictionary[0]);

} // namespace tests

} // namespace ucanopen


