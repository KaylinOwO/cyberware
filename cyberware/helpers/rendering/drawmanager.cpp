#include "drawmanager.h"
#include <mutex>

CDrawManager gDrawManager;

void Font::Create()
{
	MenuText = g_VGuiSurface->CreateFont_();
	Velocity = g_VGuiSurface->CreateFont_();
	DamageIndicator = g_VGuiSurface->CreateFont_();
	ESP = g_VGuiSurface->CreateFont_();
	Name = g_VGuiSurface->CreateFont_();

	g_VGuiSurface->SetFontGlyphSet(MenuText, "Tahoma", 12, 800, 0, 0, FONTFLAG_OUTLINE);
	g_VGuiSurface->SetFontGlyphSet(Velocity, "Verdana", 20, 800, 0, 0, FONTFLAG_OUTLINE | FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(DamageIndicator, "Verdana", 15, 800, 0, 0, FONTFLAG_OUTLINE | FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(ESP, "Arial", 12, 700, 0, 0, FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(Name, "Tahoma", 12, 700, 0, 0, FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(Flags, "Small Fonts", 8, 400, 0, 0, FONTFLAG_OUTLINE);
}


//===================================================================================
void CDrawManager::Initialize()
{
	if (g_VGuiSurface == NULL)
		return;

	g_EngineClient->GetScreenSize(g_ScreenSizeW, g_ScreenSizeH);
}
//===================================================================================
void CDrawManager::DrawLine(int x, int y, int x1, int y1, Color clrColor)
{
	g_VGuiSurface->DrawSetColor(clrColor.r(), clrColor.g(), clrColor.b(), clrColor.a());
	g_VGuiSurface->DrawLine(x, y, x1, y1);
}
//===================================================================================
void CDrawManager::DrawString(int x, int y, Color color, vgui::HFont font, const wchar_t* pszText)
{
	if (pszText == NULL)
		return;

	g_VGuiSurface->DrawSetTextPos(x, y);
	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->DrawPrintText(pszText, wcslen(pszText));
}
//===================================================================================
void CDrawManager::DrawString(int x, int y, Color color, vgui::HFont font, const wchar_t* pszText, TextAlignment alignment)
{
	if (pszText == NULL)
		return;

	int width, height;
	g_VGuiSurface->GetTextSize(font, pszText, width, height);
	if (alignment != TextAlignment::AlignmentNone) {
		if (alignment == TextAlignment::AlignmentCenter)
			x -= width / 2;
		else if (alignment == TextAlignment::AlignmentRight)
			x -= width;
	}

	g_VGuiSurface->DrawSetTextPos(x, y);
	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->DrawPrintText(pszText, wcslen(pszText));
}
//===================================================================================
void CDrawManager::DrawString(int x, int y, Color color, vgui::HFont font, const char* pszText, ...)
{
	if (!pszText)
		return;

	va_list va_alist;
	char szBuffer[1024] = { '\0' };
	wchar_t szString[1024] = { '\0' };

	va_start(va_alist, pszText);
	vsprintf_s(szBuffer, pszText, va_alist);
	va_end(va_alist);

	wsprintfW(szString, L"%S", szBuffer);

	g_VGuiSurface->DrawSetTextPos(x, y);
	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->DrawPrintText(szString, wcslen(szString));
}
//===================================================================================
void CDrawManager::DrawString(int x, int y, Color col, vgui::HFont font, const char* text, TextAlignment alignment) {
	size_t original_size = strlen(text) + 1;
	const size_t new_size = 1024;
	size_t converted_characters = 0;
	wchar_t wcstring[new_size];
	mbstowcs_s(&converted_characters, wcstring, original_size, text, _TRUNCATE);

	int width, height;
	g_VGuiSurface->GetTextSize(font, wcstring, width, height);

	if (alignment != TextAlignment::AlignmentNone) {
		if (alignment == TextAlignment::AlignmentCenter)
			x -= width / 2;
		else if (alignment == TextAlignment::AlignmentRight)
			x -= width;
	}

	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(col.r(), col.g(), col.b(), col.a());
	g_VGuiSurface->DrawSetTextPos(x, y);
	g_VGuiSurface->DrawPrintText(wcstring, wcslen(wcstring));
}
//===================================================================================

//Thank you kolo.
std::wstring CDrawManager::StringToWide(const std::string& text)
{
	std::wstring wide(text.length(), L' ');
	std::copy(text.begin(), text.end(), wide.begin());

	return wide;
}

int CDrawManager::GetWidth(unsigned long font, const char* input)
{
	INT iWide = 0;
	INT iTall = 0;
	INT iBufSize = MultiByteToWideChar(CP_UTF8, 0x0, input, -1, NULL, 0);

	wchar_t* pszUnicode = new wchar_t[iBufSize];

	MultiByteToWideChar(CP_UTF8, 0x0, input, -1, pszUnicode, iBufSize);

	g_VGuiSurface->GetTextSize(font, pszUnicode, iWide, iTall);

	delete[] pszUnicode;

	return iWide;
}

void CDrawManager::DrawStringA(unsigned long font, bool center, int x, int y, int r, int g, int b, int a, const char* input, ...)
{
	CHAR szBuffer[MAX_PATH];

	if (!input)
		return;

	vsprintf(szBuffer, input, (char*)&input + _INTSIZEOF(input));

	if (center)
		x -= GetWidth(font, szBuffer) / 2;

	g_VGuiSurface->DrawSetTextColor(r, g, b, a);
	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextPos(x, y);
	std::wstring wide = StringToWide(std::string(szBuffer));
	g_VGuiSurface->DrawPrintText(wide.c_str(), wide.length());
}

//===================================================================================
void CDrawManager::DrawT(int X, int Y, Color Color, vgui::HFont Font, bool Center, const char* _Input, ...)
{
	char Buffer[2048] = { '\0' };
	va_list Args;

	va_start(Args, _Input);
	vsprintf_s(Buffer, _Input, Args);
	va_end(Args);

	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];

	mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

	int Width = 0, Height = 0;

	if (Center)
		g_VGuiSurface->GetTextSize(Font, WideBuffer, Width, Height);

	g_VGuiSurface->DrawSetTextColor(Color.r(), Color.g(), Color.b(), Color.a());
	g_VGuiSurface->DrawSetTextFont(Font);
	g_VGuiSurface->DrawSetTextPos(X, Y);
	g_VGuiSurface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
}
//===================================================================================
void CDrawManager::DrawRect(int x, int y, int w, int h, Color clrColor)
{
	g_VGuiSurface->DrawSetColor(clrColor.r(), clrColor.g(), clrColor.b(), clrColor.a());
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);
}
//===================================================================================
void CDrawManager::OutlineRect(int x, int y, int w, int h, Color clrColor)
{
	g_VGuiSurface->DrawSetColor(clrColor.r(), clrColor.g(), clrColor.b(), clrColor.a());
	g_VGuiSurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void CDrawManager::OutlineCircle(int x, int y, int r, int seg, Color color)
{
	g_VGuiSurface->DrawSetColor(color);
	g_VGuiSurface->DrawOutlinedCircle(x, y, r, seg);
}

//===================================================================================
bool CDrawManager::WorldToScreen(const Vector& vOrigin, Vector& vScreen)
{
	return (g_DebugOverlay->ScreenPosition(vOrigin, vScreen) != 1);
}
