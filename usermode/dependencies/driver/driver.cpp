#include "driver.h"

int m_ue::interface_t::get_process_id(std::string proc_name)
{
	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (stricmp(entry.szExeFile, proc_name.c_str()) == 0)
			{
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	return 0;
}

bool m_ue::interface_t::start_service( ) {
	this->m_handle = CreateFileA( _( "\\\\.\\{37581902-15CD-4FCB-B17F-A7515AD33274}" ), GENERIC_READ, 0, 0, 3, 0x00000080, 0 );

	if ( this->m_handle != INVALID_HANDLE_VALUE ) {
		return true;
	}

	return false;
}

bool m_ue::interface_t::is_mapped( int proc ) {
	this->m_pid = proc;
	if ( !m_pid ) {
		return false;
	}

	return true;
}

bool m_ue::interface_t::resolve_dtb( ) {
	dtb_invoke data{ 0 };

	data.pid = this->m_pid;

	send_cmd( &data, invoke_resolve_dtb );

	return data.operation;
}

bool m_ue::interface_t::send_cmd( void* data, requests code ) {
	if ( !data || !code )
		return false;

	IO_STATUS_BLOCK block;

	invoke_data request{ 0 };

	request.unique = invoke_unique;
	request.data = data;
	request.code = code;

	DirectIO( this->m_handle, nullptr, nullptr, nullptr, &block, ( ULONG )nullptr, &request, sizeof( request ), &request, sizeof( request ) );

	return true;
}

std::uintptr_t m_ue::interface_t::get_module_base( const char* module_name ) {
	base_invoke data{ 0 };

	data.pid = this->m_pid;
	data.handle = 0;
	data.name = module_name;

	send_cmd( &data, invoke_base );

	return data.handle;
}

bool m_ue::interface_t::write( const std::uintptr_t address, void* buffer, const std::size_t size ) {
	write_invoke data{ 0 };

	data.pid = this->m_pid;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	return send_cmd( &data, invoke_write );
}

bool m_ue::interface_t::read( const std::uintptr_t address, void* buffer, const std::size_t size ) {
	read_invoke data{ 0 };

	data.pid = this->m_pid;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	return send_cmd( &data, invoke_read );
}

bool m_ue::interface_t::dump_memory(uint64_t address, size_t len)
{
	for (int i = 0; i < len; i+=sizeof(float))
	{
		float buffer;
		read(address + i, &buffer, sizeof(float));

		if (std::abs(buffer) < std::numeric_limits<float>::epsilon() || buffer > 100000)
			continue;

		//printf("offset: 0x%p, value %f\n", i, buffer);
	}
	Sleep(5);
	system("cls");

	return false;
}
