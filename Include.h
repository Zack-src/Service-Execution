
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iomanip>


__int64 Get_Service_PID(const char* name);
__int64 privilege(const char* priv);
void Get_PcaSvc_File(HANDLE hConsole);



std::vector<std::string> extract_paths(const std::string& input);
bool file_exists(const std::string& path);
std::vector<DWORD> get_all_process_ids();
std::string find_pcaclient(HANDLE process_handle);
std::string get_process_name(DWORD process_id);
std::string get_service_name(DWORD process_id);
void getLastLaunchTime(const std::string& path);