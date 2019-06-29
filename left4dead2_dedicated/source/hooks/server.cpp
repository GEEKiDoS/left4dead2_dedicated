#include "stdafx.hpp"
#include "hooks.hpp"

static std::unique_ptr<PLH::x86Detour> g_pCServerGameDLLGetTickIntervalHook;
static uint64_t g_CServerGameDLLGetTickIntervalOrig = NULL;

double hkCServerGameDLLGetTickInterval()
{
	float tickinterval = 0.03333333f; // Ä¬ÈÏÊÇ 1/30 (30tick)

	if (CommandLine()->CheckParm("-tickrate"))
	{
		float tickrate = CommandLine()->ParmValue("-tickrate", 0.0f);
		if (tickrate > 10.0f)
			tickinterval = 1.0f / tickrate;
	}

	return tickinterval;
}

void OnServerLoaded(const uintptr_t dwServerBase)
{
	static bool bHasLoaded = false;

	if (bHasLoaded)
	{
		return;
	}

	bHasLoaded = true;

	PLH::CapstoneDisassembler dis(PLH::Mode::x86);
	
	g_pCServerGameDLLGetTickIntervalHook = SetupDetourHook(
		dwServerBase + 0xE8FC0, &hkCServerGameDLLGetTickInterval, &g_CServerGameDLLGetTickIntervalOrig, dis);

	g_pCServerGameDLLGetTickIntervalHook->hook();
}