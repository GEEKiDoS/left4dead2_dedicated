#include "stdafx.hpp"
#include "hooks.hpp"

#include <istream>

class Color
{
public:
    // constructors
    Color() { *( (int*)this ) = 0; }
    Color( int _r, int _g, int _b ) { SetColor( _r, _g, _b, 0 ); }
    Color( int _r, int _g, int _b, int _a ) { SetColor( _r, _g, _b, _a ); }

    // set the color
    // r - red component (0-255)
    // g - green component (0-255)
    // b - blue component (0-255)
    // a - alpha component, controls transparency (0 - transparent, 255 -
    // opaque);
    void SetColor( int _r, int _g, int _b, int _a = 0 )
    {
        _color[0] = (unsigned char)_r;
        _color[1] = (unsigned char)_g;
        _color[2] = (unsigned char)_b;
        _color[3] = (unsigned char)_a;
    }

    void GetColor( int& _r, int& _g, int& _b, int& _a ) const
    {
        _r = _color[0];
        _g = _color[1];
        _b = _color[2];
        _a = _color[3];
    }

    void SetRawColor( int color32 ) { *( (int*)this ) = color32; }

    int GetRawColor() const { return *( (int*)this ); }

    inline int r() const { return _color[0]; }
    inline int g() const { return _color[1]; }
    inline int b() const { return _color[2]; }
    inline int a() const { return _color[3]; }

    unsigned char& operator[]( int index ) { return _color[index]; }

    const unsigned char& operator[]( int index ) const { return _color[index]; }

    bool operator==( const Color& rhs ) const
    {
        return ( *( (int*)this ) == *( (int*)&rhs ) );
    }

    bool operator!=( const Color& rhs ) const { return !( operator==( rhs ) ); }

    Color& operator=( const Color& rhs )
    {
        SetRawColor( rhs.GetRawColor() );
        return *this;
    }

private:
    unsigned char _color[4];
};

static std::unique_ptr<PLH::x86Detour> g_pColorPrintHook;
static uint64_t g_ColorPrintOrig = NULL;

void __cdecl hkCon_ColorPrint( Color& clr, const char* msg )
{
    if ( clr.r() > clr.g() + clr.b() )  // Error
    {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 12 );
        printf( "%s", msg );
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7 );
    }
    else if ( clr.r() + clr.g() > clr.g() + clr.b() )  // Warn
    {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 14 );
        printf( "%s", msg );
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7 );
    }
    else if ( clr.g() > clr.r() + clr.b() )  // Green message idk
    {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 10 );
        printf( "%s", msg );
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7 );
    }
    else
    {
        if ( strstr( msg, "Unknown command" ) ||
             strstr( msg, "not found" ) ||
             strstr( msg, "WARN" ) )
        {
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 14 );
        }

		if (strstr(msg, "successful"))
		{
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 10 );
		}

        printf( "%s", msg );

        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7 );
    }

    return PLH::FnCast( g_ColorPrintOrig, &hkCon_ColorPrint )( clr, msg );
}

static std::unique_ptr<PLH::x86Detour> g_pCOM_ParseLineHook;
static uint64_t g_COM_ParseLineOrig = NULL;

char* com_token;

char* hkCOM_ParseLine(char* data)
{
	int c;
	int len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

	c = *data;

	// parse a line out of the data
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	} while ((c != '\n' && c != 0) && (len < 1023));

	if (*(data - 1) == '\r')
		len--;
	
	com_token[len] = 0;

	if (c == 0) // end of file
		return NULL;

	return data;
}

static std::unique_ptr<PLH::x86Detour> g_pSetDataRateHook;
static uint64_t g_SetDataRateOrig = NULL;

void __fastcall hkSetDataRate(uintptr_t ecx, void* edx, float rate)
{
	// ȡ�� clampping
	*(float*)(ecx + 0xB0) = rate; // m_Rate = rate;
}

void OnEngineLoaded( const uintptr_t dwEngineBase )
{
    static bool bHasLoaded = false;

    if ( bHasLoaded )
    {
        return;
    }

    bHasLoaded = true;

	com_token = (char *)dwEngineBase + 0x686458;

	PLH::CapstoneDisassembler dis( PLH::Mode::x86 );

    g_pColorPrintHook = SetupDetourHook(
        dwEngineBase + 0xA33F0, &hkCon_ColorPrint, &g_ColorPrintOrig, dis );

	g_pCOM_ParseLineHook = SetupDetourHook(
		dwEngineBase + 0x156C40, &hkCOM_ParseLine, &g_COM_ParseLineOrig, dis);

	g_pSetDataRateHook = SetupDetourHook(
		dwEngineBase + 0x1AE9D0, &hkSetDataRate, &g_SetDataRateOrig, dis);

    g_pColorPrintHook->hook();
	g_pCOM_ParseLineHook->hook();
	g_pSetDataRateHook->hook();
}
