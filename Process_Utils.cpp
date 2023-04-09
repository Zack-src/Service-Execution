#include "Include.h"

__int64 Get_Service_PID(const char* name) 
{

	auto shandle = OpenSCManagerA(0, 0, 0),
		shandle_ = OpenServiceA(shandle, name, SERVICE_QUERY_STATUS);

	if (!shandle || !shandle_) return 0;

	SERVICE_STATUS_PROCESS ssp{}; DWORD bytes;

	bool query = QueryServiceStatusEx(shandle_, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytes);

	CloseServiceHandle(shandle);
	CloseServiceHandle(shandle_);
	return ssp.dwProcessId;

}

__int64 privilege(const char* priv) 
{

	HANDLE thandle;
	LUID identidier;
	TOKEN_PRIVILEGES privileges{};

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &thandle)) return 0;

	if (!LookupPrivilegeValue(0, priv, &identidier)) return 0;

	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identidier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(thandle, 0, &privileges, sizeof(privileges), NULL, NULL))
		return 0;

	CloseHandle(thandle); return 1;

}


/*chatGPT*/

std::vector<std::string> extract_paths(const std::string& input) {
	std::vector<std::string> paths;
	size_t pos = 0;
	while ((pos = input.find(",MonitorProcess,", pos)) != std::string::npos) {
		size_t start = pos + 16;
		size_t end = input.find(',', start);
		std::string path = input.substr(start, end - start);
		// Vérifier si le chemin commence par une lettre de lecteur valide
		if (path.length() >= 3 && isalpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) {
			paths.push_back(path);
		}
		pos = end;
	}
	return paths;
}

bool file_exists(const std::string& path) {
	return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

std::vector<DWORD> get_all_process_ids() {
	std::vector<DWORD> process_ids;

	DWORD process_array[4096], needed_bytes;
	if (EnumProcesses(process_array, sizeof(process_array), &needed_bytes)) {
		DWORD process_count = needed_bytes / sizeof(DWORD);
		for (DWORD i = 0; i < process_count; i++) {
			if (process_array[i] != 0) {
				process_ids.push_back(process_array[i]);
			}
		}
	}

	return process_ids;
}

std::string find_pcaclient(HANDLE process_handle) {
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	MEMORY_BASIC_INFORMATION mbi;
	LPCVOID address = sys_info.lpMinimumApplicationAddress;

	while (address < sys_info.lpMaximumApplicationAddress) {
		if (VirtualQueryEx(process_handle, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
			if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE) && !(mbi.Protect & PAGE_GUARD)) {
				std::string buffer(mbi.RegionSize, '\0');
				SIZE_T bytesRead;
				if (ReadProcessMemory(process_handle, mbi.BaseAddress, &buffer[0], mbi.RegionSize, &bytesRead)) {
					buffer.resize(bytesRead);
					size_t pos = buffer.find("TRACE,");
					if (pos != std::string::npos) {
						return buffer.substr(pos);
					}
				}
			}
			address = (LPBYTE)mbi.BaseAddress + mbi.RegionSize;
		}
		else {
			break;
		}
	}

	return "";
}

std::string get_process_name(DWORD process_id) {
	HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
	if (process_handle) {
		CHAR process_name[MAX_PATH];
		GetModuleBaseNameA(process_handle, NULL, process_name, MAX_PATH);
		CloseHandle(process_handle);
		return std::string(process_name);
	}
	return "";
}

std::string get_service_name(DWORD process_id) {
	SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	if (sc_manager) {
		DWORD bytes_needed;
		DWORD service_count;
		EnumServicesStatusExA(sc_manager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytes_needed, &service_count, NULL, NULL);
		std::vector<BYTE> buffer(bytes_needed);
		EnumServicesStatusExA(sc_manager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, buffer.data(), bytes_needed, &bytes_needed, &service_count, NULL, NULL);
		LPENUM_SERVICE_STATUS_PROCESSA services = (LPENUM_SERVICE_STATUS_PROCESSA)buffer.data();
		for (DWORD i = 0; i < service_count; ++i) {
			if (services[i].ServiceStatusProcess.dwProcessId == process_id) {
				CloseServiceHandle(sc_manager);
				return std::string(services[i].lpServiceName);
			}
		}
		CloseServiceHandle(sc_manager);
	}
	return "";
}

/* end chatGPT*/