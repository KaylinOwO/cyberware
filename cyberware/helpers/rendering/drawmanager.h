#pragma once
//===================================================================================
#include "../../valve_sdk/sdk.hpp"

#define RED(COLORCODE)	((int) ( COLORCODE >> 24) )
#define BLUE(COLORCODE)	((int) ( COLORCODE >> 8 ) & 0xFF )
#define GREEN(COLORCODE)	((int) ( COLORCODE >> 16 ) & 0xFF )
#define ALPHA(COLORCODE)	((int) COLORCODE & 0xFF )
#define COLORCODE(r,g,b,a)((DWORD)((((r)&0xff)<<24)|(((g)&0xff)<<16)|(((b)&0xff)<<8)|((a)&0xff)))

enum TextAlignment {
	AlignmentNone = 0,
	AlignmentCenter,
	AlignmentRight
};


typedef unsigned long HFont;
class Font : public Singleton<Font>
{
public:
	HFont MenuText, Velocity, DamageIndicator, ESP, Name, Flags;
	void Create();
};

//===================================================================================
class CDrawManager
{
public:
	void Initialize();
	void DrawLine(int x, int y, int x1, int y1, Color clrColor);
	void DrawString(int x, int y, Color color, vgui::HFont font, const wchar_t* pszText);
	void DrawString(int x, int y, Color color, vgui::HFont font, const wchar_t* pszText, TextAlignment alignment);
	void DrawString(int x, int y, Color color, vgui::HFont font, const char* pszText, ...);
	void DrawString(int x, int y, Color col, vgui::HFont font, const char* text, TextAlignment alignment);
	std::wstring StringToWide(const std::string& text);
	int GetWidth(unsigned long font, const char* input);
	void DrawStringA(unsigned long font, bool center, int x, int y, int r, int g, int b, int a, const char* input, ...);
	void DrawT(int X, int Y, Color Color, vgui::HFont Font, bool Center, const char* _Input, ...);
	std::byte GetESPHeight();
	void DrawRect(int x, int y, int w, int h, Color clrColor);
	void OutlineRect(int x, int y, int w, int h, Color clrColor);
	void OutlineCircle(int x, int y, int r, int seg, Color color);
	int GetPixelTextSize(const char* pszText);
	int GetPixelTextSize(wchar_t* pszText);
	bool WorldToScreen(const Vector& vOrigin, Vector& vScreen);
	DWORD dwGetTeamColor(int iIndex)
	{
		static DWORD dwColors[] = { 0, //Dummy
					 0, // 1 Teamone (UNUSED)
					 0xFF8000FF, // 2 Teamtwo (RED)
					 0x0080FFFF, // 3 teamthree (BLUE)
					 0 // 4 Teamfour (UNUSED) 
		};
		return dwColors[iIndex];
	}

};
//===================================================================================
extern CDrawManager gDrawManager;
//===================================================================================