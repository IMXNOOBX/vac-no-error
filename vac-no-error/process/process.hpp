#include <windows.h>
#include <tlhelp32.h>

#define size(arr) sizeof(arr) / sizeof(arr[0])

namespace proc {
	const char* steamProcesses[] = { "csgo.exe", "Steam.exe", "SteamClient.exe", "SteamService.exe", "SteamWebHelper.exe", "steamwebhelper.exe" };

	bool terminate_process(const char* pName) {
		PROCESSENTRY32 processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		// Create a snapshot of the current running processes
		HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (processSnapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		// Iterate through the running processes
		if (Process32First(processSnapshot, &processEntry)) {
			do {
				if (_stricmp(processEntry.szExeFile, pName) == 0) {
					// Get the process ID and open the process
					DWORD processId = processEntry.th32ProcessID;
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);

					if (hProcess != NULL) {
						// Terminate the process
						if (TerminateProcess(hProcess, 0)) {
							CloseHandle(hProcess);
							CloseHandle(processSnapshot);
							return true;
						}

						CloseHandle(hProcess);
					}
				}
			} while (Process32Next(processSnapshot, &processEntry));
		}

		// Clean up and return false if the process was not found or couldn't be terminated
		CloseHandle(processSnapshot);
		return false;
	}

	bool check_open_processname(const char* pName) {
		PROCESSENTRY32 processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		bool isOpen = false;
		// Create a snapshot of the current running processes
		HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (processSnapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		// Iterate through the running processes
		if (Process32First(processSnapshot, &processEntry)) {
			do {
				if (_stricmp(processEntry.szExeFile, pName) == 0) {
					isOpen = true;
				}
			} while (Process32Next(processSnapshot, &processEntry));
		}

		// Clean up and return false if the process was not found or couldn't be terminated
		CloseHandle(processSnapshot);

		return isOpen;
	}
}