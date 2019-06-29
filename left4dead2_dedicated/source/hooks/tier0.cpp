#include "stdafx.hpp"
#include "hooks.hpp"

void OnTier0Loaded(const uintptr_t dwTier0Base)
{
	static bool bHasLoaded = false;

	if (bHasLoaded)
	{
		return;
	}

	bHasLoaded = true;
}