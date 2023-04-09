#include "Include.h"

int main() {


	SetConsoleTitleA("Service-Execution, by github.com/Zack-src");
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	privilege(SE_DEBUG_NAME);

	std::vector<DWORD> process_ids = get_all_process_ids();

	for (DWORD process_id : process_ids) {
		HANDLE process_handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, process_id);
		if (process_handle) {
			std::string pcaclient_content = find_pcaclient(process_handle);
			CloseHandle(process_handle);

			if (!pcaclient_content.empty()) {
				std::vector<std::string> paths = extract_paths(pcaclient_content);

				if (!paths.empty()) {
					std::string process_name = get_process_name(process_id);
					std::string service_name = process_name == "svchost.exe" ? get_service_name(process_id) : "";

					std::cout << process_name;
					if (!service_name.empty()) {
						std::cout << " (Service: " << service_name << ")";
					}
					std::cout << " (" << process_id << ")" << std::endl;

					for (const std::string& path : paths) {
						if (file_exists(path))
						{
							SetConsoleTextAttribute(hConsole, 2);
							std::cout << "\tFile is present   ";
						}
						else
						{
							SetConsoleTextAttribute(hConsole, 4);
							std::cout << "\tFile is deleted   ";
						}
						SetConsoleTextAttribute(hConsole, 7);
						std::cout << path << std::endl;
					}

					std::cout << std::endl;
				}
			}
		}
	}

	Get_PcaSvc_File(hConsole);


	std::cout << "\n\n" << "------End------";

	std::cin.ignore();
	return 0;
}