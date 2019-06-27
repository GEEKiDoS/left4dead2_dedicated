#include "stdafx.hpp"
#include "hooks.hpp"

static std::unique_ptr<PLH::x86Detour> g_pSysWriteStatusTextHook;
static uint64_t g_SysWriteStatusTextOrig = NULL;

int ConvertUtf8ToWideChar( const char* szInput, wchar_t* szOutput,
                           const std::uint32_t iOutBufferSize )
{
    int iLength = MultiByteToWideChar( CP_UTF8, NULL, szInput, -1, szOutput,
                                       iOutBufferSize );
    szOutput[iOutBufferSize - 1] = L'\0';
    return iLength;
}

wchar_t szConsoleTitle[1024] = { 0 };

void WINAPI hkSysWriteStatusText( char* szText )
{
    // 修复控制台标题乱码
    ConvertUtf8ToWideChar( szText, szConsoleTitle, 1024 );
    SetConsoleTitleW( szConsoleTitle );
}

static std::unique_ptr<PLH::x86Detour> g_pInitConProcHook;
static uint64_t g_InitConProcOrig = NULL;

void hkInitConProc( void )
{
    // 修复控制台输出乱码
    SetConsoleCP( CP_UTF8 );
    SetConsoleOutputCP( CP_UTF8 );

    freopen( "CONOUT$", "w", stdout );
    freopen( "CONOUT$", "w", stderr );

    return PLH::FnCast( g_InitConProcOrig, hkInitConProc )();
}

static std::unique_ptr<PLH::x86Detour> g_pPrintRawHook;
static uint64_t g_PrintRawOrig = NULL;

void __fastcall hkPrintRaw( void* ecx, void* edx, char* pszMsg, int nChars )
{
    if ( nChars == 1 || strlen( pszMsg ) == 1 )
        return PLH::FnCast( g_PrintRawOrig, hkPrintRaw )( ecx, edx, pszMsg, nChars );
}

static std::unique_ptr<PLH::x86Detour> g_pSetStatusLineHook;
static uint64_t g_SetStatusLineOrig = NULL;

void __fastcall hkSetStatusLine(void* ecx, void* edx, char* pszStatus)
{
    wchar_t szOutput[1024] = { 0 };
    ConvertUtf8ToWideChar( pszStatus, szOutput, 1024 );

    wchar_t szBuffer[1024] = { 0 };
    _snwprintf( szBuffer, 1024, L"%s - [%s]", szConsoleTitle, szOutput );
    SetConsoleTitleW( szBuffer );
}

void BytePatchDedicated( const uintptr_t dwDedicatedBase ) 
{
    //
    // 强制控制台模式
    //
    // nop nop
    const std::array<uint8_t, 2> fcPatch = { 0x90, 0x90 };
    utils::WriteProtectedMemory( fcPatch, dwDedicatedBase + 0x3814 );

	//
	// 免除 -game 参数
	//
	// push "left4dead2"

	union
	{
		uint32_t addr = reinterpret_cast<uint32_t>("left4dead2");
		uint8_t bytes[4];
	} l4d2;

	const std::array<uint8_t, 4> gamePatch = { l4d2.bytes[0], l4d2.bytes[1], l4d2.bytes[2], l4d2.bytes[3] };
	utils::WriteProtectedMemory(gamePatch, dwDedicatedBase + 0x38AD);
}

void OnDedicatedLoaded( const uintptr_t dwDedicatedBase )
{
    static bool bHasLoaded = false;

    if ( bHasLoaded )
    {
        return;
    }

    bHasLoaded = true;

	BytePatchDedicated( dwDedicatedBase );

    PLH::CapstoneDisassembler dis( PLH::Mode::x86 );

    g_pSysWriteStatusTextHook =
        SetupDetourHook( dwDedicatedBase + 0x5B70, &hkSysWriteStatusText,
                         &g_SysWriteStatusTextOrig, dis );

    g_pInitConProcHook = SetupDetourHook(
        dwDedicatedBase + 0x4E50, &hkInitConProc, &g_InitConProcOrig, dis );

	g_pPrintRawHook = SetupDetourHook( dwDedicatedBase + 0x59E0, &hkPrintRaw,
                                       &g_PrintRawOrig, dis );

	g_pSetStatusLineHook = SetupDetourHook(
        dwDedicatedBase + 0x5AA0, &hkSetStatusLine, &g_SetStatusLineOrig, dis );

    g_pSysWriteStatusTextHook->hook();
	g_pInitConProcHook->hook();
    g_pPrintRawHook->hook();
    g_pSetStatusLineHook->hook();
}