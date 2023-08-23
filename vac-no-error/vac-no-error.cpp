#include <iostream>
#include <thread>

#include "process/process.hpp"
#include "utils/debug.hpp"

int main()
{
	HKEY hKey;
	WCHAR steam_username[256]; // Adjust the size according to your needs
	DWORD steam_username_s = sizeof(steam_username);
	WCHAR steam_path_w[256];
	DWORD steam_path_s = sizeof(steam_path_w);

	SetConsoleTitleA("vac no error");

	debug::log::out("Welcome to VAC Error fix tool!\n\n");

	debug::log::out("Checking if CS:GO is open...\n");

	bool csgo_open = proc::check_open_processname("csgo.exe");
	if (csgo_open)
		debug::log::success("CS:GO was open, it will reopen automatically.");

	for (int i = 0; i < sizear(proc::steamProcesses); i++)
		proc::terminate_process(proc::steamProcesses[i]);

	debug::log::success("Steam has been closed.");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	LONG openRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_ALL_ACCESS, &hKey);
	if (openRes != ERROR_SUCCESS) {
		debug::log::error("Could not open the Steam registry key.");
		return 0;
	}

	debug::log::success("Opened the Steam registry key.");

	LONG getPath = RegQueryValueExW(hKey, L"SteamPath", 0, NULL, (LPBYTE)steam_path_w, &steam_path_s);
	if (getPath != ERROR_SUCCESS) {
		debug::log::error("Could not read the SteamPath.");
		return 0;
	}

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, steam_path_w, -1, nullptr, 0, nullptr, nullptr);
	std::string steam_path(size_needed - 1, 0); // Subtract 1 to exclude the null character
	WideCharToMultiByte(CP_UTF8, 0, steam_path_w, -1, &steam_path[0], size_needed, nullptr, nullptr);

	LONG getName = RegQueryValueExW(hKey, L"AutoLoginUser", 0, NULL, (LPBYTE)steam_username, &steam_username_s);
	if (getName != ERROR_SUCCESS) {
		debug::log::error("Could not read the AutoLoginUser username.");
		return 0;
	}

	debug::log::success("Read and cached the current Steam username.");

	LONG clearName = RegSetValueExW(hKey, L"AutoLoginUser", 0, REG_SZ, (LPBYTE)L"", sizeof(L""));
	if (clearName != ERROR_SUCCESS) {
		debug::log::error("Could not reset AutoLoginUser.");
		return 0;
	}

	debug::log::success("Cleared the AutoLoginUser username.");

	std::string delete_files[] = { "Steam.dll", "SteamUI.dll", "clientregistry.blob" };

	debug::log::warn("Deleting (3) files to restore them later...");

	for (int i = 0; i < sizear(delete_files); i++)
		std::remove((steam_path + "\\" + delete_files[i]).c_str());

	HINSTANCE hInstance = ShellExecute(NULL, "open", (steam_path + "\\bin\\steamservice.exe").c_str(), NULL, NULL, SW_SHOWNORMAL);
	if ((int)hInstance <= 32) {
		debug::log::error("Could not open steamservice repair.");
		return 0;
	}

	hInstance = ShellExecute(NULL, "open", "steam://open/main", NULL, NULL, SW_SHOWNORMAL);
	if ((int)hInstance <= 32) {
		debug::log::error("Could not open steamservice repair.");
		return 0;
	}

	debug::log::success("Waiting until the Steam login prompt opens.");

	while (!FindWindowA("vguiPopupWindow", NULL)) // Chrome_RenderWidgetHostHWND
		std::this_thread::sleep_for(std::chrono::seconds(3));

	debug::log::success("Opened, closing Steam.");

	for (int i = 0; i < sizear(proc::steamProcesses); i++)
		proc::terminate_process(proc::steamProcesses[i]);

	debug::log::success("Steam has been closed.");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	LONG resetName = RegSetValueExW(hKey, L"AutoLoginUser", 0, REG_SZ, (LPBYTE)steam_username, steam_username_s);
	if (resetName != ERROR_SUCCESS) {
		debug::log::error("Could not set AutoLoginUser to the old username.");
		return 0;
	}

	debug::log::success("Reset AutoLoginUser to the cached username.");

	DWORD rem = 1; // Set the value to 1 to enable RememberPassword
	LONG rememberPass = RegSetValueExW(hKey, L"RememberPassword", 0, REG_DWORD, reinterpret_cast<BYTE*>(&rem), sizeof(rem));
	if (rememberPass != ERROR_SUCCESS) {
		debug::log::error("Could not enable RememberPassword.");
		return 0;
	}

	debug::log::success("Reset enabled RememberPassword.");

	hInstance = ShellExecuteW(NULL, L"open", (csgo_open ? L"steam://rungameid/730" : L"steam://open/main"), NULL, NULL, SW_SHOWNORMAL);
	if ((int)hInstance <= 32) {
		debug::log::warn("Could not open Steam.");
	}

	debug::log::success(csgo_open ? "Opening CS:GO back again.\n" : "Opening Steam back again.\n");

	RegCloseKey(hKey);

	debug::log::out("VAC Error should be fixed.");

	std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}
