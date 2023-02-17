#include "ucanopen_tests.h"


namespace ucanopen {

namespace tests {

Object* Server::_object;
unsigned char object_alloc[sizeof(Object)] __attribute__((section("shared_ucanopen_test_object")));


namespace od {

SdoAbortCode get_device_name(ExpeditedSdoData& retval)
{
	const size_t len = strlen(sysinfo::device_name) + 1;
	const size_t word_count = (len + 3) / 4;
	static size_t counter = 0;

	char word[4] = {0};
	strncpy(word, sysinfo::device_name + 4*counter, 4);
	emb::c28x::from_bytes<uint32_t>(retval.u32, reinterpret_cast<uint16_t*>(word));

	counter = (counter + 1) % word_count;

	return SdoAbortCode::no_error;
}


SdoAbortCode get_hardware_version(ExpeditedSdoData& retval)
{
	const size_t len = strlen(sysinfo::hardware_version) + 1;
	const size_t word_count = (len + 3) / 4;
	static size_t counter = 0;

	char word[4] = {0};
	strncpy(word, sysinfo::hardware_version + 4*counter, 4);
	emb::c28x::from_bytes<uint32_t>(retval.u32, reinterpret_cast<uint16_t*>(word));

	counter = (counter + 1) % word_count;

	return SdoAbortCode::no_error;
}


SdoAbortCode get_firmware_version(ExpeditedSdoData& retval)
{
	const size_t len = strlen(sysinfo::firmware_version) + 1;
	const size_t word_count = (len + 3) / 4;
	static size_t counter = 0;

	char word[4] = {0};
	strncpy(word, sysinfo::firmware_version + 4*counter, 4);
	emb::c28x::from_bytes<uint32_t>(retval.u32, reinterpret_cast<uint16_t*>(word));

	counter = (counter + 1) % word_count;

	return SdoAbortCode::no_error;
}


SdoAbortCode save_all_parameters(ExpeditedSdoData val)
{
	return SdoAbortCode::no_error;
}


SdoAbortCode restore_all_default_parameters(ExpeditedSdoData val)
{
	return SdoAbortCode::no_error;
}


SdoAbortCode get_serial_number(ExpeditedSdoData& retval)
{
	uint32_t* uid_ptr = reinterpret_cast<uint32_t*>(0x000703CC);
	retval.u32 = *uid_ptr;
	return SdoAbortCode::no_error;
}


inline SdoAbortCode reset_device(ExpeditedSdoData val)
{
	syslog::add_message(sys::Message::device_software_resetting);
	mcu::chrono::system_clock::register_delayed_task(mcu::reset_device, 2000);
	return SdoAbortCode::no_error;
}


inline SdoAbortCode reset_errors(ExpeditedSdoData val)
{
	syslog::reset_errors_warnings();
	return SdoAbortCode::no_error;
}


inline SdoAbortCode get_syslog_message(ExpeditedSdoData& retval)
{
	retval.u32 = syslog::read_message().underlying_value();
	syslog::pop_message();
	return SdoAbortCode::no_error;
}

inline SdoAbortCode get_uptime(ExpeditedSdoData& retval)
{
	retval.f32 = mcu::chrono::system_clock::now() / 1000.f;
	return SdoAbortCode::no_error;
}


uint32_t parameter_1;
float parameter_2;
uint32_t parameter_3;
float parameter_4;
uint32_t parameter_5;

} // namespace od


ODEntry object_dictionary[] = {
{{0x1008, 0x00}, {"sys", "info", "device_name", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_device_name, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x1009, 0x00}, {"sys", "info", "hardware_version", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_hardware_version, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x100A, 0x00}, {"sys", "info", "firmware_version", "", OD_STRING, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_firmware_version, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x1010, 0x01}, {"sys", "ctl", "save_all_parameters", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::save_all_parameters}},
{{0x1011, 0x01}, {"sys", "ctl", "restore_all_default_parameters", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::restore_all_default_parameters}},

{{0x1018, 0x04}, {"sys", "info", "serial_number", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_serial_number, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2000, 0x01}, {"sys", "ctl", "reset_device", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::reset_device}},
{{0x2000, 0x02}, {"sys", "ctl", "reset_errors", "", OD_EXEC, OD_ACCESS_WO, OD_NO_DIRECT_ACCESS, OD_NO_INDIRECT_READ_ACCESS, od::reset_errors}},

{{0x5000, 0x01}, {"watch", "watch", "uptime", "s", OD_FLOAT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_uptime, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5000, 0x02}, {"watch", "watch", "syslog_message", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::get_syslog_message, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x3000, 0x01}, {"config", "group 1", "parameter 1", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&od::parameter_1), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3000, 0x02}, {"config", "group 1", "parameter 2", "", OD_FLOAT32, OD_ACCESS_RW, OD_PTR(&od::parameter_2), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3001, 0x01}, {"config", "group 2", "parameter 3", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&od::parameter_3), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3001, 0x02}, {"config", "group 2", "parameter 4", "", OD_FLOAT32, OD_ACCESS_RW, OD_PTR(&od::parameter_4), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x3001, 0x03}, {"config", "group 2", "parameter 5", "", OD_UINT32, OD_ACCESS_RW, OD_PTR(&od::parameter_5), OD_NO_INDIRECT_READ_ACCESS, OD_NO_INDIRECT_WRITE_ACCESS}},
};


const size_t object_dictionary_size = sizeof(object_dictionary) / sizeof(object_dictionary[0]);

} // namespace tests

} // namespace ucanopen


