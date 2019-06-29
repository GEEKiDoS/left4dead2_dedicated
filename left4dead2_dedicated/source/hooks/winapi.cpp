#include "stdafx.hpp"

#include "hooks.hpp"

extern void OnEngineLoaded(const uintptr_t dwEngineBase);
extern void OnTier0Loaded(const uintptr_t dwTier0Base);
extern void OnServerLoaded(const uintptr_t dwServerBase);

void OnLoadLibrary(HMODULE hLibrary, std::string_view libPathView)
{
	// convert the library path to just the library file name
	std::filesystem::path libPath = libPathView;
	const std::string szLibName = libPath.filename().string();

	const uintptr_t dwLibraryBase = reinterpret_cast<uintptr_t>(hLibrary);

	if (szLibName == "engine.dll")
	{
		OnEngineLoaded(dwLibraryBase);
	}
	else if (szLibName == "tier0.dll")
	{
		OnTier0Loaded(dwLibraryBase);
	}
	else if (szLibName == "server.dll")
	{
		OnServerLoaded(dwLibraryBase);
	}
}

static std::unique_ptr<PLH::x86Detour> g_pLoadLibExHook;
static uint64_t g_LoadLibExOrig = NULL;

NOINLINE HMODULE WINAPI hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile,
	DWORD dwFlags)
{
	HMODULE hLoadedModule = PLH::FnCast(g_LoadLibExOrig, hkLoadLibraryExA)(
		lpLibFileName, hFile, dwFlags);
	if(hLoadedModule)
		OnLoadLibrary(hLoadedModule, lpLibFileName);
	return hLoadedModule;
}

void HookWinapi()
{
	PLH::CapstoneDisassembler dis(PLH::Mode::x86);

	g_pLoadLibExHook =
		SetupDetourHook(reinterpret_cast<uintptr_t>(&LoadLibraryExA),
			&hkLoadLibraryExA, &g_LoadLibExOrig, dis);
	g_pLoadLibExHook->hook();
}
