#include <dependencies/includes.hpp>

struct ClientClass {
	uint64_t pCreateFn;
	uint64_t pCreateEventFn;
	uint64_t pNetworkName;
	uint64_t pRecvTable;
	uint64_t pNext;
	uint32_t ClassID;
	uint32_t ClassSize;
};

std::string get_class_name(uint64_t entity_ptr)
{
	uint64_t client_networkable_vtable = device_t.read<uint64_t>(entity_ptr + 8 * 3);
	if (!client_networkable_vtable)
		return std::string();

	uint64_t get_client_class = device_t.read<uint64_t>(client_networkable_vtable + 8 * 3);
	if (!get_client_class)
		return std::string();

	uint32_t disp = device_t.read<uint32_t>(get_client_class + 3);
	if (!disp)
		return std::string();

	const uint64_t client_class_ptr = get_client_class + disp + 7;
	if (!client_class_ptr)
		return std::string();

	ClientClass client_class = device_t.read<ClientClass>(client_class_ptr);
	if (!client_class.ClassID)
		return std::string();

	char name[32] = {};
	device_t.read_array<char>(client_class.pNetworkName, name, 32);

	return std::string(name);
}

#define OFFSET_ENTITYLIST			0x1e743a8

#define OFFSET_HIGHLIGHTSETTINGS 0xade5c40
#define OFFSET_HIGHLIGHTSERVERACTIVESTATES  0x298      //OFF_GLOW_HIGHLIGHT_ID 
#define OFFSET_HIGHLIGHTCURRENTCONID  0x28C       // OFF_GLOW_ENABLE 
#define OFFSET_HIGHLIGHTVISIBILITYTYPE  0x26c       //OFF_GLOW_THROUGH_WALL 0x278
#define HIGHLIGHT_TYPE_SIZE 0x34 //? updated 01/10/2024
#define OFFSET_GLOW_COLOR 0x1d0	+ 0x30				// m_highlightParams + 0x18
#define OFFSET_GLOW_TYPE 0x29c				// m_highlightParams + 0x18
#define OFFSET_GLOW_FIX   0x268

struct HighlightFunctionBits
{
	uint8_t functionBits[4];
};

struct HighlightParameter
{
	float parameter[3];
};

void SetColor(DWORD64 entity, int idx, uint8_t settingIndex, HighlightFunctionBits setting, HighlightParameter color)
{

	device_t.write<int>(entity + OFFSET_HIGHLIGHTVISIBILITYTYPE, 2);
	device_t.write<int>(entity + OFFSET_HIGHLIGHTCURRENTCONID, idx);
	device_t.write<uint8_t>(entity + idx + OFFSET_HIGHLIGHTSERVERACTIVESTATES, settingIndex);
	uint64_t HighlightSettings = device_t.read<uint64_t>(device_t.m_base + OFFSET_HIGHLIGHTSETTINGS);

	device_t.write<HighlightFunctionBits>(HighlightSettings + 0x34 * uint64_t(settingIndex) + 0, setting);
	device_t.write<HighlightParameter>(HighlightSettings + 0x34 * uint64_t(settingIndex) + 4, color);

	device_t.write(entity + 0x268, 2);

	return;
}

int main( )
{
	if (!device_t.start_service()) {
		m_log("[-] Driver not loaded\n");
		return 0;
	}

	device_t.m_pid = device_t.get_process_id("r5apex.exe");
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
	
	const auto entity_list = device_t.m_base + OFFSET_ENTITYLIST;

	while (1)
	{
		for (int i = 0; i < 10000; i++)
		{
			uint64_t entity = device_t.read<uint64_t>(entity_list + ((uint64_t)i << 5));
			if (!entity)
				continue;

			const auto name = get_class_name(entity);
			if (name.empty())
				continue;

			//printf("Name: %s\n", name.c_str());

			if (name == "CPlayer")
			{
				SetColor(entity, 5, 80, { 0,101,64,64 }, { 0, 30, 0 });

			}
		}
	}
}