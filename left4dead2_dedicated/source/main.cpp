#include "stdafx.hpp"

typedef int (*DedicatedMain_t)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// Must add 'bin' to the path....
	std::string path = getenv("PATH");

	// Use the .EXE name to determine the root directory
	char moduleName[MAX_PATH];
	if (!GetModuleFileName(hInstance, moduleName, MAX_PATH))
	{
		MessageBox(0, "Failed calling GetModuleFileName", "Launcher Error", MB_OK);
		return 0;
	}

	// Get the root directory the .exe is in
	auto rootDir = utils::GetBaseDir(std::filesystem::path(moduleName)) / "bin";

	auto newPath = "PATH=" + rootDir.string() + ";" + path;
	if (_putenv(newPath.c_str()) == -1)
	{
		MessageBox(0, "Failed to set enviroment variable", "Launcher Error", MB_OK);

		return 0;
	}

	auto launcher = LoadLibrary("bin\\dedicated.dll"); // STEAM OK ... filesystem not mounted yet
	if (!launcher)
	{
		char* pszError;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& pszError, 0, NULL);

		std::string errorMessage = "Failed to load the launcher DLL:\n\n" + std::string(pszError);
		MessageBox(0, errorMessage.c_str(), "Launcher Error", MB_OK);

		LocalFree(pszError);
		return 0;
	}

	DedicatedMain_t main = (DedicatedMain_t)GetProcAddress(launcher, "DedicatedMain");

	return main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}
