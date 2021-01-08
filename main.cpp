//https://github.com/diegcrane

#include <string>
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>

__int64 Get_Service_PID(const char* name) {

	auto shandle = OpenSCManagerA(0, 0, 0),
		shandle_ = OpenServiceA(shandle, name, SERVICE_QUERY_STATUS);

	if (!shandle || !shandle_) return 0;

	SERVICE_STATUS_PROCESS ssp{}; DWORD bytes;

	bool query = QueryServiceStatusEx(shandle_, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytes);

	CloseServiceHandle(shandle);
	CloseServiceHandle(shandle_);
	return ssp.dwProcessId;

}

__int64 privilege(const char* priv) {

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

std::vector<std::string> Get_Memory_Execution() {

	__int64 pid = Get_Service_PID("PcaSvc");
	if (!pid) return { "0" };

	HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (!phandle) return { "0" };

	std::vector<std::string>list; MEMORY_BASIC_INFORMATION info;

	for (static __int64 address = 0; VirtualQueryEx(phandle, (LPVOID)address, &info, sizeof(info)); address += info.RegionSize)
	{
		if (info.State != MEM_COMMIT) continue;

		std::string memory;
		memory.resize(info.RegionSize);

		if (!ReadProcessMemory(phandle, (LPVOID)address, &memory[0], info.RegionSize, 0)) continue;

		for (__int64 pos = 0;
			pos != std::string::npos;
			pos = memory.find(":\\", pos + 1))
		{
			std::string path;

			for (__int64 x = pos - 1; memory[x] > 32 && memory[x] < 123; x++)
				path.push_back(memory[x]);

			if (path[path.length() - 4] != '.') {
				continue;
			}

			list.push_back(path);
		}
	}
	return list;
}

inline bool exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int main() {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!privilege(SE_DEBUG_NAME)) return 0;

	std::vector<std::string>executions = Get_Memory_Execution();
	if (executions[0] == "0" || !executions.size()) {
		return 0;
	}

	std::map<std::string, int> Lmap;
	for (std::string x : executions) {
		if (Lmap[x] == NULL) {

			Lmap[x] = 1;

			if (exists(x)) {
				SetConsoleTextAttribute(hConsole, 2);
				std::cout << "File is present   ";
			}
			else {
				SetConsoleTextAttribute(hConsole, 4);
				std::cout << "File is deleted   ";
			}
			SetConsoleTextAttribute(hConsole, 7);
			std::cout << x << std::endl;
		}
	}

	std::cin.ignore();
	return 0;
}
