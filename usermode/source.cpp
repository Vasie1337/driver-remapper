#include <dependencies/includes.hpp>

int main( )
{
	if (!device_t.start_service()) {
		m_log("[-] Driver not loaded\n");
		return 0;
	}

	device_t.m_pid = device_t.get_process_id("explorer.exe");
	if (!device_t.m_pid || !device_t.is_mapped(device_t.m_pid)) {
		m_log("[-] Target not mapped\n");
		return 0;
	}

	if (!device_t.resolve_dtb()) {
		m_log("[-] failed to get dtb\n");
		return FALSE;
	}

	device_t.m_base = device_t.get_module_base(0);
	if (!device_t.m_base) {
		m_log("[-] Failed to get base\n");
		return 0;
	}
	m_log("[+] Base: %p\n", reinterpret_cast<void*>(device_t.m_base));
	
}