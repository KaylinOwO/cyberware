#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "features/item_definitions.h"
#include "features/kit_parser.h"
#include "features/skins.h"
#include "render.hpp"
void ReadDirectory(const std::string& name, std::vector<std::string>& v)
{
	auto pattern(name);
	pattern.append("\\*.ini");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			v.emplace_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
}
struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan2("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan2("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}
namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}
	inline bool ColorEdit4a(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}

	inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}
bool Tab(const char* label, const ImVec2& size_arg, bool state)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);
	if (pressed)
		ImGui::MarkItemEdited(id);

	ImGui::RenderFrame(bb.Min, bb.Max, state ? ImColor(15, 15, 15) : ImColor(23, 23, 23), true, style.FrameRounding);
	window->DrawList->AddRect(bb.Min, bb.Max, ImColor(0, 0, 0));
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	if (state)
	{
		window->DrawList->AddLine(bb.Min, bb.Min + ImVec2(9, 0), ImColor(255, 255, 255), 1);
		window->DrawList->AddLine(bb.Min, bb.Min + ImVec2(0, 9), ImColor(255, 255, 255), 1);

		window->DrawList->AddLine(bb.Max - ImVec2(0, 1), bb.Max - ImVec2(10, 1), ImColor(255, 255, 255), 1);
		window->DrawList->AddLine(bb.Max - ImVec2(1, 1), bb.Max - ImVec2(1, 10), ImColor(255, 255, 255), 1);
	}

	return pressed;
}
void Menu::SpectatorList()
{
	if (!g_Options.spectator_list)
		return;

	std::string spectators;

	if (g_EngineClient->IsInGame() && g_LocalPlayer)
	{
		for (int i = 1; i <= g_GlobalVars->maxClients; i++)
		{
			auto ent = C_BasePlayer::GetPlayerByIndex(i);

			if (!ent || ent->IsAlive() || ent->IsDormant())
				continue;

			auto target = (C_BasePlayer*)ent->m_hObserverTarget();

			if (!target || target != g_LocalPlayer)
				continue;

			if (ent == target)
				continue;

			auto info = ent->GetPlayerInfo();

			spectators += std::string(info.szName) + u8"\n";
		}
	}

	if (ImGui::Begin("Spectator List", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |( _visible ? NULL : ImGuiWindowFlags_NoMove)))
	{
		ImGui::PushFont(g_SpectatorListFont);

		ImGui::Text("Spectator List");

		ImGui::Text(spectators.c_str());
		ImGui::PopFont();

	}
	ImGui::End();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;
	SpectatorList();
    if(!_visible)
        return;
	auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | NULL | NULL | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | NULL | NULL | NULL;

	static int tab = 0;
	static int visuals_page = 1;
	/*
	ImGui::Begin("Dear ImGui Style Editor");
	ImGui::ShowStyleEditor();
	ImGui::End();
	*/
	ImGui::SetNextWindowSize({ 560.000000f,360.000000f });

	ImGui::Begin("edited", nullptr, flags);
	{
		ImVec2 p = ImGui::GetWindowPos();
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 550, p.y + 350), ImColor(0.000000f, 0.000000f, 0.000000f, 0.4f), 0, 15); // main frame
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 0, p.y + 0), ImVec2(p.x + 560, p.y + 360), ImColor(0.000000f, 0.000000f, 0.000000f, 0.639216f), 0, 15, 1.000000);  // main frame
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 550, p.y + 350), ImColor(0.000000f, 0.000000f, 0.000000f, 0.639216f), 0, 15, 1.000000);  // main frame
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 15, p.y + 60), ImVec2(p.x + 545, p.y + 345), ImColor(0.066667f, 0.066667f, 0.066667f, 1.000000f), 0, 15); // main frame
		ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + 15, p.y + 60), ImVec2(p.x + 545, p.y + 345), ImColor(0.000001f, 0.000001f, 0.000001f, 1.000000f), 0, 15, 1.000000);  // main frame

		ImGui::SetCursorPos({ 15,17 });
		if (Tab("legitbot", { 125,35 }, tab == 0))
			tab = 0;

		ImGui::SetCursorPos({ 15 + 125 + 10,17 });
		if (Tab("visuals", { 125,35 }, tab == 1))
			tab = 1;

		ImGui::SetCursorPos({ 15 + 250 + 20,17 });
		if (Tab("misc", { 125,35 }, tab == 2))
			tab = 2;

		ImGui::SetCursorPos({ 15 + 375 + 30,17 });
		if (Tab("skins", { 125,35 }, tab == 3))
			tab = 3;

		if (tab == 0)
		{
			static int definition_index = WEAPON_INVALID;

			auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
			if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
				definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
			else
				definition_index = WEAPON_INVALID;
			if (definition_index == WEAPON_INVALID)definition_index = WEAPON_DEAGLE;
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{
				ImGui::Separator("general");
				auto settings = &g_Options.weapons[definition_index].legit;

				ImGui::Checkbox("enabled", &g_Options.aim_enabled);
				ImGui::Combo("weapon", &g_Options.aim_weaponselect, "pistol\0rifle\0sniper\0scout\0smg\0heavy \0");
				//ImGui::Checkbox("Friendly fire", &settings->deathmatch);
				ImGui::Checkbox("flash check", &g_Options.aim_flash_check);
				ImGui::Checkbox("smoke check", &g_Options.aim_smoke_check);
				ImGui::Checkbox("auto-pistol", &g_Options.aim_autopistol);


			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Separator("settings");

				const char* aim_types[] = {
						"type: static",
						"type: closest"
				};

				static char* hitbox_list[] = { "head", "neck", "pelvis", "stomach", "lower chest", "chest", "upper chest" };


				switch (g_Options.aim_weaponselect)
				{
					case 0: //pistol
					{
						ImGui::Text("  fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##fov", &g_Options.aim_fov_pistol, 0.f, 20.f, "%.f");
						ImGui::Spacing();
						ImGui::Checkbox("silent", &g_Options.aim_silent_pistol);
						ImGui::Text("  smooth");
						ImGui::Spacing();
						ImGui::SliderFloat("##smooth", &g_Options.aim_smooth_pistol, 0.f, 20.f, "%.f");

						if (ImGui::BeginCombo("##aimtype", aim_types[g_Options.aim_type_pistol], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(aim_types); i++)
							{
								if (ImGui::Selectable(aim_types[i], i == g_Options.aim_type_pistol))
									g_Options.aim_type_pistol = i;
							}

							ImGui::EndCombo();
						}

						if (g_Options.aim_type_pistol == 0)
						{
							if (ImGui::BeginCombo("##aimhitbox", hitbox_list[g_Options.aim_hitbox_pistol], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(hitbox_list); i++)
								{
									if (ImGui::Selectable(hitbox_list[i], i == g_Options.aim_hitbox_pistol))
										g_Options.aim_hitbox_pistol = i;
								}

								ImGui::EndCombo();
							}
						}

						ImGui::Checkbox("autowall##pistol", &g_Options.aim_autowall_pistol);
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::Text("min damage");
						ImGui::Spacing();
						ImGui::SliderInt("##mindamagepistol", &g_Options.aim_mindamage_pistol, 1, 100, "%i");

						ImGui::Separator("rcs");

						ImGui::Checkbox("enabled##rcs", &g_Options.aim_rcs_pistol);

						const char* rcs_types[] = {
							"type: standalone",
							"type: aim"
						};


						if (ImGui::BeginCombo("##type", rcs_types[g_Options.aim_rcs_type_pistol], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
							{
								if (ImGui::Selectable(rcs_types[i], i == g_Options.aim_rcs_type_pistol))
									g_Options.aim_rcs_type_pistol = i;
							}

							ImGui::EndCombo();
						}
						//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
						ImGui::SliderInt("  x", &g_Options.aim_rcs_x_pistol, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
						ImGui::SliderInt("  y", &g_Options.aim_rcs_y_pistol, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();

						break;
					}
					case 1: //rifle
					{
						ImGui::Text("  fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##fov", &g_Options.aim_fov_rifle, 0.f, 20.f, "%.f");
						ImGui::Spacing();
						ImGui::Checkbox("silent", &g_Options.aim_silent_rifle);
						ImGui::Text("  smooth");
						ImGui::Spacing();
						ImGui::SliderFloat("##smooth", &g_Options.aim_smooth_rifle, 0.f, 20.f, "%.f");
						if (ImGui::BeginCombo("##aimtype", aim_types[g_Options.aim_type_rifle], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(aim_types); i++)
							{
								if (ImGui::Selectable(aim_types[i], i == g_Options.aim_type_rifle))
									g_Options.aim_type_rifle = i;
							}

							ImGui::EndCombo();
						}

						if (g_Options.aim_type_rifle == 0)
						{
							if (ImGui::BeginCombo("##aimhitbox", hitbox_list[g_Options.aim_hitbox_rifle], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(hitbox_list); i++)
								{
									if (ImGui::Selectable(hitbox_list[i], i == g_Options.aim_hitbox_rifle))
										g_Options.aim_hitbox_rifle = i;
								}

								ImGui::EndCombo();
							}
						}
						ImGui::Checkbox("autowall##rifle", &g_Options.aim_autowall_rifle);
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::Text("min damage");
						ImGui::Spacing();
						ImGui::SliderInt("##mindamagerifle", &g_Options.aim_mindamage_rifle, 1, 100, "%i");

						ImGui::Separator("rcs");

						ImGui::Checkbox("enabled##rcs", &g_Options.aim_rcs_rifle);

						const char* rcs_types[] = {
							"type: standalone",
							"type: aim"
						};


						if (ImGui::BeginCombo("##type", rcs_types[g_Options.aim_rcs_type_rifle], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
							{
								if (ImGui::Selectable(rcs_types[i], i == g_Options.aim_rcs_type_rifle))
									g_Options.aim_rcs_type_rifle = i;
							}

							ImGui::EndCombo();
						}
						//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
						ImGui::SliderInt("  x", &g_Options.aim_rcs_x_rifle, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
						ImGui::SliderInt("  y", &g_Options.aim_rcs_y_rifle, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();

						break;
					}
					case 2: //sniper
					{
						ImGui::Text("  fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##fov", &g_Options.aim_fov_sniper, 0.f, 20.f, "%.f");
						ImGui::Spacing();
						ImGui::Checkbox("silent", &g_Options.aim_silent_sniper);
						ImGui::Text("  smooth");
						ImGui::Spacing();
						ImGui::SliderFloat("##smooth", &g_Options.aim_smooth_sniper, 0.f, 20.f, "%.f");
						if (ImGui::BeginCombo("##aimtype", aim_types[g_Options.aim_type_sniper], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(aim_types); i++)
							{
								if (ImGui::Selectable(aim_types[i], i == g_Options.aim_type_sniper))
									g_Options.aim_type_sniper = i;
							}

							ImGui::EndCombo();
						}

						if (g_Options.aim_type_sniper == 0)
						{
							if (ImGui::BeginCombo("##aimhitbox", hitbox_list[g_Options.aim_hitbox_sniper], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(hitbox_list); i++)
								{
									if (ImGui::Selectable(hitbox_list[i], i == g_Options.aim_hitbox_sniper))
										g_Options.aim_hitbox_sniper = i;
								}

								ImGui::EndCombo();
							}
						}
						ImGui::Checkbox("autowall##sniper", &g_Options.aim_autowall_sniper);
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::Text("min damage");
						ImGui::Spacing();
						ImGui::SliderInt("##mindamagesniper", &g_Options.aim_mindamage_sniper, 1, 100, "%i");

						ImGui::Separator("rcs");

						ImGui::Checkbox("enabled##rcs", &g_Options.aim_rcs_sniper);

						const char* rcs_types[] = {
							"type: standalone",
							"type: aim"
						};


						if (ImGui::BeginCombo("##type", rcs_types[g_Options.aim_rcs_type_sniper], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
							{
								if (ImGui::Selectable(rcs_types[i], i == g_Options.aim_rcs_type_sniper))
									g_Options.aim_rcs_type_sniper = i;
							}

							ImGui::EndCombo();
						}
						//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
						ImGui::SliderInt("  x", &g_Options.aim_rcs_x_sniper, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
						ImGui::SliderInt("  y", &g_Options.aim_rcs_y_sniper, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();

	
						break;
					}
					case 3: //scout
					{
						ImGui::Text("  fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##fov", &g_Options.aim_fov_scout, 0.f, 20.f, "%.f");
						ImGui::Spacing();
						ImGui::Checkbox("silent", &g_Options.aim_silent_scout);
						ImGui::Text("  smooth");
						ImGui::Spacing();
						ImGui::SliderFloat("##smooth", &g_Options.aim_smooth_scout, 0.f, 20.f, "%.f");

						if (ImGui::BeginCombo("##aimtype", aim_types[g_Options.aim_type_scout], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(aim_types); i++)
							{
								if (ImGui::Selectable(aim_types[i], i == g_Options.aim_type_scout))
									g_Options.aim_type_scout = i;
							}

							ImGui::EndCombo();
						}

						if (g_Options.aim_type_scout == 0)
						{
							if (ImGui::BeginCombo("##aimhitbox", hitbox_list[g_Options.aim_hitbox_scout], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(hitbox_list); i++)
								{
									if (ImGui::Selectable(hitbox_list[i], i == g_Options.aim_hitbox_scout))
										g_Options.aim_hitbox_scout = i;
								}

								ImGui::EndCombo();
							}
						}

						ImGui::Checkbox("autowall##scout", &g_Options.aim_autowall_scout);
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::Text("min damage");
						ImGui::Spacing();
						ImGui::SliderInt("##mindamagescout", &g_Options.aim_mindamage_smg, 1, 100, "%i");
						break;
					}
					case 4: //smg 
					{
						ImGui::Text("  fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##fov", &g_Options.aim_fov_smg, 0.f, 20.f, "%.f");
						ImGui::Spacing();
						ImGui::Checkbox("silent", &g_Options.aim_silent_smg);
						ImGui::Text("  smooth");
						ImGui::Spacing();
						ImGui::SliderFloat("##smooth", &g_Options.aim_smooth_smg, 0.f, 20.f, "%.f");
						if (ImGui::BeginCombo("##aimtype", aim_types[g_Options.aim_type_smg], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(aim_types); i++)
							{
								if (ImGui::Selectable(aim_types[i], i == g_Options.aim_type_smg))
									g_Options.aim_type_smg = i;
							}

							ImGui::EndCombo();
						}

						if (g_Options.aim_type_smg == 0)
						{
							if (ImGui::BeginCombo("##aimhitbox", hitbox_list[g_Options.aim_hitbox_smg], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(hitbox_list); i++)
								{
									if (ImGui::Selectable(hitbox_list[i], i == g_Options.aim_hitbox_smg))
										g_Options.aim_hitbox_smg = i;
								}

								ImGui::EndCombo();
							}
						}

						ImGui::Checkbox("autowall##smg", &g_Options.aim_autowall_heavy);
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::Text("min damage");
						ImGui::Spacing();
						ImGui::SliderInt("##mindamagesmg", &g_Options.aim_mindamage_smg, 1, 100, "%i");

						ImGui::Separator("rcs");

						ImGui::Checkbox("enabled##rcs", &g_Options.aim_rcs_smg);

						const char* rcs_types[] = {
							"type: standalone",
							"type: aim"
						};


						if (ImGui::BeginCombo("##type", rcs_types[g_Options.aim_rcs_type_smg], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
							{
								if (ImGui::Selectable(rcs_types[i], i == g_Options.aim_rcs_type_smg))
									g_Options.aim_rcs_type_smg = i;
							}

							ImGui::EndCombo();
						}
						//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
						ImGui::SliderInt("  x", &g_Options.aim_rcs_x_smg, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
						ImGui::SliderInt("  y", &g_Options.aim_rcs_y_smg, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();

						break;
					}
					case 5:
					{
						ImGui::Text("  fov");
						ImGui::Spacing();
						ImGui::SliderFloat("##fov", &g_Options.aim_fov_heavy, 0.f, 20.f, "%.f");
						ImGui::Spacing();
						ImGui::Checkbox("silent", &g_Options.aim_silent_heavy);
						ImGui::Text("  smooth");
						ImGui::Spacing();
						ImGui::SliderFloat("##smooth", &g_Options.aim_smooth_heavy, 0.f, 20.f, "%.f");
						if (ImGui::BeginCombo("##aimtype", aim_types[g_Options.aim_type_heavy], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(aim_types); i++)
							{
								if (ImGui::Selectable(aim_types[i], i == g_Options.aim_type_heavy))
									g_Options.aim_type_heavy = i;
							}

							ImGui::EndCombo();
						}

						if (g_Options.aim_type_heavy == 0)
						{
							if (ImGui::BeginCombo("##aimhitbox", hitbox_list[g_Options.aim_hitbox_heavy], ImGuiComboFlags_NoArrowButton))
							{
								for (int i = 0; i < IM_ARRAYSIZE(hitbox_list); i++)
								{
									if (ImGui::Selectable(hitbox_list[i], i == g_Options.aim_hitbox_heavy))
										g_Options.aim_hitbox_heavy = i;
								}

								ImGui::EndCombo();
							}
						}


						ImGui::Checkbox("autowall##heavy", &g_Options.aim_autowall_heavy);
						ImGui::Spacing();
						ImGui::SameLine();
						ImGui::Text("min damage");
						ImGui::Spacing();
						ImGui::SliderInt("##mindamageheavy", &g_Options.aim_mindamage_heavy, 1, 100, "%i");

						ImGui::Separator("rcs");

						ImGui::Checkbox("enabled##rcs", &g_Options.aim_rcs_heavy);

						const char* rcs_types[] = {
							"type: standalone",
							"type: aim"
						};


						if (ImGui::BeginCombo("##type", rcs_types[g_Options.aim_rcs_type_heavy], ImGuiComboFlags_NoArrowButton))
						{
							for (int i = 0; i < IM_ARRAYSIZE(rcs_types); i++)
							{
								if (ImGui::Selectable(rcs_types[i], i == g_Options.aim_rcs_type_heavy))
									g_Options.aim_rcs_type_heavy = i;
							}

							ImGui::EndCombo();
						}
						//ImGui::SliderInt("##start", &settings->rcs.start, 1, 30, "Start: %i");
						ImGui::SliderInt("  x", &g_Options.aim_rcs_x_heavy, 0, 100, "%i");
						ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
						ImGui::SliderInt("  y", &g_Options.aim_rcs_y_heavy, 0, 100, "%i");

						break;
					}
				}
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				auto settings = &g_Options.weapons[definition_index].legit;



				/*
				ImGui::Separator("auto-fire");
				ImGui::Checkbox("enabled##autofire", &settings->autofire.enabled);
				ImGui::SameLine(group_w - 50);
				ImGui::Hotkey("##autofire", &settings->autofire.hotkey);
				*/
				ImGui::Separator("triggerbot");
				ImGui::Checkbox("enabled##triggerbot", &g_Options.trigger_enabled); ImGui::SameLine(group_w - 50);  ImGui::Hotkey("##triggerbotkey", &g_Options.trigger_key);
				ImGui::Text("delay");
				ImGui::Spacing();
				ImGui::SliderInt("##triggerdelay", &g_Options.trigger_delay, 0, 10);
				ImGui::Spacing();
				if (ImGui::BeginCombo("##trigger_hitbox_filter", "hitboxes", ImGuiComboFlags_NoArrowButton))
				{
					ImGui::Selectable("head", &g_Options.trigger_head, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("chest", &g_Options.trigger_chest, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("stomach", &g_Options.trigger_stomach, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("arms", &g_Options.trigger_arms, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("legs", &g_Options.trigger_legs, ImGuiSelectableFlags_DontClosePopups);

					ImGui::EndCombo();
				}

				ImGui::Separator("backtrack");
				ImGui::Checkbox("enabled##backtrack", &g_Options.misc_bt);
				ImGui::Checkbox("extended##backtrack", &g_Options.extended_bt);
				if (!g_Options.extended_bt)
					ImGui::SliderInt("##backtrackticks", &g_Options.bt_ticks, 1, 200, "%i");
			}
			ImGui::EndChild();
		}
		else if (tab == 1)
		{
			if (visuals_page == 1)
			{
				ImGui::SetCursorPos({ 21,65 });
				ImGui::BeginChild("##1", { 166,276 });
				{
					ImGui::Separator("esp");
					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

					ImGui::Checkbox("enabled", &g_Options.esp_enabled);
					ImGui::Checkbox("visible only", &g_Options.esp_visonly);
					ImGui::Checkbox("boxes", &g_Options.esp_player_boxes); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4("enemies visible   ", &g_Options.color_esp_enemy);
					ImGui::Checkbox("names", &g_Options.esp_player_names);
					ImGui::Checkbox("health", &g_Options.esp_player_health);
					ImGui::Checkbox("weapon", &g_Options.esp_player_weapons);
				}
				ImGui::EndChild();

				ImGui::SetCursorPos({ 31 + 166,65 });
				ImGui::BeginChild("##2", { 166,276 });
				{
					ImGui::Separator("chams");

					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

					ImGui::Checkbox("enabled ", &g_Options.chams_player_enabled); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("enemy visible ", &g_Options.color_chams_player_enemy_visible);
					ImGui::Checkbox("visible shine##chams_enemies_visible_shine", &g_Options.player_enemies_shine);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4("##color_chams_enemies_visible_shine", &g_Options.player_enemy_visible_shine);

					ImGui::Checkbox("occluded  ", &g_Options.chams_player_ignorez); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("enemy occluded ", &g_Options.color_chams_player_enemy_occluded);

					ImGui::Combo("##flat", &g_Options.chams_player_flat, "normal\0flat \0");
					ImGui::Checkbox("backtrack chams ", &g_Options.chams_player_backtrack); ImGui::SameLine(group_w - 20); ImGuiEx::ColorEdit4a("enemy backtrack ", &g_Options.color_chams_backtrack);
					ImGui::Checkbox("draw all ticks ", &g_Options.chams_player_backtrack_allticks);
					ImGui::Combo("##backtracktype", &g_Options.chams_player_backtrack_type, "normal\0flat \0");

				}
				ImGui::EndChild();

				ImGui::SetCursorPos({ 41 + 166 + 166,65 });
				ImGui::BeginChild("##3", { 166,166 });
				{
					ImGui::Separator("glow");

					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;
					ImGui::Checkbox("enabled", &g_Options.glow_enabled);
					ImGui::Checkbox("players", &g_Options.glow_players);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4a("##enemy   ", &g_Options.color_glow_enemy);
					ImGui::Checkbox("occluded   ", &g_Options.glow_enemiesOC);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4a("##color_glow_enemiesOC   ", &g_Options.color_glow_enemyOC);
					const char* glow_enemies_type[] = {
						"outline outer",
						"pulse",
						"outline inner"
					};
					if (ImGui::BeginCombo("##glow_enemies_type", "visible type", ImGuiComboFlags_NoArrowButton))
					{
						for (int i = 0; i < IM_ARRAYSIZE(glow_enemies_type); i++)
						{
							if (ImGui::Selectable(glow_enemies_type[i], i == g_Options.glow_enemies_type))
								g_Options.glow_enemies_type = i;
						}

						ImGui::EndCombo();
					}//
					ImGui::Checkbox("dropped weapons", &g_Options.glow_weapons);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4a("##weapong   ", &g_Options.color_glow_weapons);
					ImGui::Checkbox("c4", &g_Options.glow_planted_c4);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4a("##c4g   ", &g_Options.color_glow_planted_c4);

				}
				ImGui::EndChild();
			}

			if (visuals_page == 2)
			{
				ImGui::SetCursorPos({ 21,65 });
				ImGui::BeginChild("##1", { 166,276 });
				{
					ImGui::Separator("world");

					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;
					ImGui::Checkbox("night mode", &g_Options.world_modulation);
					ImGui::Checkbox("no flash", &g_Options.no_flash);
					ImGui::Checkbox("no smoke", &g_Options.no_smoke);
					//ImGui::Checkbox("Glass",&g_Options.chams_player_glass);
					//ImGuiEx::ColorEdit4("Ally (Visible)",&g_Options.color_chams_player_ally_visible);
					//ImGuiEx::ColorEdit4("Ally (Occluded)",&g_Options.color_chams_player_ally_occluded);

				}
				ImGui::EndChild();

				ImGui::SetCursorPos({ 31 + 166,65 });
				ImGui::BeginChild("##2", { 166,276 });
				{
					ImGui::Separator("misc");

					float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

					ImGui::Checkbox("rank reveal", &g_Options.misc_showranks);
					ImGui::Checkbox("spectator list", &g_Options.spectator_list);
					ImGui::Text("watermark");
					ImGui::SameLine();
					ImGui::Combo("##watermarktype", &g_Options.misc_watermark, "disabled\0normal\0simple \0");
					ImGui::Checkbox("grenade predicition", &g_Options.GrenadeIndicator);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4("##grenadeindicator", &g_Options.color_grenade_indicator);
					ImGui::Checkbox("velocity indicator", &g_Options.Velocity);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4("##velocity", &g_Options.Velocitycol);
					ImGui::Spacing();

					if (ImGui::BeginCombo("##velocity", "velocity", ImGuiComboFlags_NoArrowButton))
					{
						ImGui::Selectable("last jump", &g_Options.LastJump, ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable("graph", &g_Options.Graph, ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					ImGui::Checkbox("force crosshair", &g_Options.sniper_xhair);
					ImGui::Checkbox("killsound", &g_Options.killsound);
					ImGui::Checkbox("hitsound", &g_Options.hitsound);
					ImGui::Checkbox("hitmarker", &g_Options.hitmarker);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4("##hitmarkerc", &g_Options.color_hitmarker);
					ImGui::Checkbox("damage indicator", &g_Options.damage_indicator);
					ImGui::SameLine(group_w - 20);
					ImGuiEx::ColorEdit4("##damageindicatorc", &g_Options.color_damageindicator);
				}
				ImGui::EndChild();
			}

			ImGui::SetCursorPos({ 115 + 166 + 166,300 });
			if (Tab("1", { 35,35 }, visuals_page == 1))
				visuals_page = 1;

			ImGui::SetCursorPos({ 165 + 166 + 166,300 });
			if (Tab("2", { 35,35 }, visuals_page == 2))
				visuals_page = 2;
		}
		else if (tab == 2)
		{
			ImGui::SetCursorPos({ 21,65 });
			ImGui::BeginChild("##1", { 166,276 });
			{				
				ImGui::Separator("general");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("auto accept", &g_Options.autoaccept);
				ImGui::Checkbox("clantag", &g_Options.misc_clantag);
				if (g_Options.misc_clantag_type == 1)
				{
					(group_w - 50);
					ImGui::InputText("##clantagstring", g_Options.misc_clantag_string, 16);
				}
				ImGui::Combo("##clantagtype", &g_Options.misc_clantag_type, "cyberware\0custom \0");
				ImGui::Checkbox("desync", &g_Options.Desync); ImGui::SameLine();     ImGui::Hotkey("##desyncswitchkey", &g_Options.DesyncSwitchKey);
			//	ImGui::Combo("##desynctype", &g_Options.DesyncType, "static\0balance \0");
				ImGui::Checkbox("discord rpc", &g_Options.misc_discordrpc);
			//	ImGui::Checkbox("doubletap", &g_Options.MDoubletap);
			//	ImGui::SameLine();          ImGui::Hotkey("##doubletapkey", &g_Options.MDoubletapKey);
			//	ImGui::Checkbox("discord rpc", &g_Options.misc_discordrpc);

			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 31 + 166,65 });
			ImGui::BeginChild("##2", { 166,276 });
			{
				ImGui::Separator("movement");

				float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().FramePadding.x * 2;

				ImGui::Checkbox("auto bhop", &g_Options.misc_bhop);
				ImGui::Checkbox("auto strafe", &g_Options.autostrafe); ImGui::SameLine(group_w - 50);     ImGui::Hotkey("##autostrafekey", &g_Options.AutoStafe_key);
				ImGui::Checkbox("edgebug", &g_Options.edge_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("##edgebugkey", &g_Options.edge_bug_key);
				ImGui::Checkbox("jumpbug", &g_Options.jump_bug); ImGui::SameLine(group_w - 50);          ImGui::Hotkey("##jumpbugkey", &g_Options.jump_bug_key);
				ImGui::Checkbox("edge jump", &g_Options.edgejump.enabled); ImGui::SameLine(group_w - 50); ImGui::Hotkey("##edgejumpkey", &g_Options.edgejump.hotkey);
				ImGui::Checkbox("duck in air", &g_Options.edgejump.edge_jump_duck_in_air);
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 41 + 166 + 166,65 });
			ImGui::BeginChild("##3", { 166,276 });
			{
				ImGui::Separator("config");

				static int selected = 0;
				static char cfgName[64];

				std::vector<std::string> cfgList;
				ReadDirectory(g_Options.folder, cfgList);
				ImGui::PushItemWidth(150.f);
				if (!cfgList.empty())
				{
					if (ImGui::BeginCombo("##selectconfig", cfgList[selected].c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (size_t i = 0; i < cfgList.size(); i++)
						{
							if (ImGui::Selectable(cfgList[i].c_str(), i == selected))
								selected = i;
						}
						ImGui::EndCombo();

					}
					if (ImGui::Button("load"))
					{
						g_Options.LoadSettings(cfgList[selected]);
						static auto clear_hud_weapon_icon_fn =
							reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
								Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC"));

						auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

						if (!element)
							return;

						auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xa0);
						if (hud_weapons == nullptr)
							return;

						if (!*hud_weapons->get_weapon_count())
							return;

						for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
							i = clear_hud_weapon_icon_fn(hud_weapons, i);

						typedef void(*ForceUpdate) (void);
						static ForceUpdate FullUpdate =
							(ForceUpdate)Utils::PatternScan(GetModuleHandleA("engine.dll"), "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");
						FullUpdate();
					}
					ImGui::SameLine();
					//ImGui::SameLine();
					if (ImGui::Button("save"))
						g_Options.SaveSettings(cfgList[selected]);
					ImGui::SameLine();
					//ImGui::SameLine();
					if (ImGui::Button("delete"))
					{
						g_Options.DeleteSettings(cfgList[selected]);
						selected = 0;
					}
					//	ImGui::Separator();
				}
				ImGui::Spacing();
				ImGui::SameLine();
				ImGui::InputText("##configname", cfgName, 24);
				//ImGui::SameLine();
				if (ImGui::Button("create"))
				{
					if (strlen(cfgName))
						g_Options.SaveSettings(cfgName + std::string(".ini"));
				}
				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
		}
		else if (tab == 3)
	{
	static std::string selected_weapon_name = "";
	static std::string selected_skin_name = "";
	static auto definition_vector_index = 0;
	auto& entries = g_Options.changers.skin.m_items;
	ImGui::SetCursorPos({ 21,65 });
	ImGui::BeginChild("##1", { 166,276 });
	{
	/*	ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SameLine();*/
		//	ImGui::ListBoxHeader("##sjinstab",ImVec2(155,245));
		ImGui::Spacing();
		ImGui::Spacing();

		{
			for (size_t w = 0; w < k_weapon_names.size(); w++)
			{
				switch (w)
				{
				case 0:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "knife");
					break;
				case 2:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "glove");
					break;
				case 4:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "pistols");
					break;
				case 14:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "semi-rifle");
					break;
				case 21:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "rifle");
					break;
				case 28:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "sniper-rifle");
					break;
				case 32:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "machingun");
					break;
				case 34:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "shotgun");
					break;
				}

				if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w))
				{
					definition_vector_index = w;
				}
			}
		}
		//ImGui::ListBoxFooter();

	}
	ImGui::EndChild();

	ImGui::SetCursorPos({ 31 + 166,65 });
	ImGui::BeginChild("##2", { 166,276 });
	{			
		ImGui::Spacing();
		ImGui::Spacing();
		auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
		auto& satatt = g_Options.changers.skin.statrack_items[k_weapon_names[definition_vector_index].definition_index];
		selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
		selected_entry.definition_vector_index = definition_vector_index;
		if (selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T)
		{
			ImGui::PushItemWidth(160.f);

			ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_knife_names.at(idx).name;
					return true;
				}, nullptr, k_knife_names.size(), 10);
			selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

		}
		else if (selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE)
		{
			ImGui::PushItemWidth(160.f);

			ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_glove_names.at(idx).name;
					return true;
				}, nullptr, k_glove_names.size(), 10);
			selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
		}
		else {
			static auto unused_value = 0;
			selected_entry.definition_override_vector_index = 0;
		}

		if (selected_entry.definition_index != GLOVE_T_SIDE &&
			selected_entry.definition_index != GLOVE_CT_SIDE &&
			selected_entry.definition_index != WEAPON_KNIFE &&
			selected_entry.definition_index != WEAPON_KNIFE_T)
		{
			selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
		}
		else
		{
			if (selected_entry.definition_index == GLOVE_T_SIDE ||
				selected_entry.definition_index == GLOVE_CT_SIDE)
			{
				selected_weapon_name = k_glove_names_preview.at(selected_entry.definition_override_vector_index).name;
			}
			if (selected_entry.definition_index == WEAPON_KNIFE ||
				selected_entry.definition_index == WEAPON_KNIFE_T)
			{
				selected_weapon_name = k_knife_names_preview.at(selected_entry.definition_override_vector_index).name;
			}
		}
		if (skins_parsed)
		{
			static char filter_name[32];
			std::string filter = filter_name;

			bool is_glove = selected_entry.definition_index == GLOVE_T_SIDE ||
				selected_entry.definition_index == GLOVE_CT_SIDE;

			bool is_knife = selected_entry.definition_index == WEAPON_KNIFE ||
				selected_entry.definition_index == WEAPON_KNIFE_T;

			int cur_weapidx = 0;
			if (!is_glove && !is_knife)
			{
				cur_weapidx = k_weapon_names[definition_vector_index].definition_index;
				//selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
			}
			else
			{
				if (selected_entry.definition_index == GLOVE_T_SIDE ||
					selected_entry.definition_index == GLOVE_CT_SIDE)
				{
					cur_weapidx = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
				}
				if (selected_entry.definition_index == WEAPON_KNIFE ||
					selected_entry.definition_index == WEAPON_KNIFE_T)
				{
					cur_weapidx = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

				}
			}

			/*	ImGui::InputText("name filter [?]", filter_name, sizeof(filter_name));
				if (ImGui::ItemsToolTipBegin("##skinfilter"))
				{
					ImGui::Checkbox("show skins for selected weapon", &g_Options.changers.skin.show_cur);
					ImGui::ItemsToolTipEnd();
				}*/

			auto weaponName = weaponnames(cur_weapidx);
			/*ImGui::Spacing();

			ImGui::Spacing();
			ImGui::SameLine();*/
			//ImGui::ListBoxHeader("##sdsdadsdadas", ImVec2(155, 245));
			{
				if (selected_entry.definition_index != GLOVE_T_SIDE && selected_entry.definition_index != GLOVE_CT_SIDE)
				{
					if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
					{
						selected_entry.paint_kit_vector_index = -1;
						selected_entry.paint_kit_index = -1;
						selected_skin_name = "";
					}

					int lastID = ImGui::GetItemID();
					for (size_t w = 0; w < k_skins.size(); w++)
					{
						for (auto names : k_skins[w].weaponName)
						{
							std::string name = k_skins[w].name;

							if (g_Options.changers.skin.show_cur)
							{
								if (names != weaponName)
									continue;
							}

							if (name.find(filter) != name.npos)
							{
								ImGui::PushID(lastID++);

								ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(is_knife && g_Options.changers.skin.show_cur ? 6 : k_skins[w].rarity));
								{
									if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
									{
										selected_entry.paint_kit_vector_index = w;
										selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
										selected_skin_name = k_skins[w].name_short;
									}
								}
								ImGui::PopStyleColor();

								ImGui::PopID();
							}
						}
					}
				}
				else
				{
					int lastID = ImGui::GetItemID();

					if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
					{
						selected_entry.paint_kit_vector_index = -1;
						selected_entry.paint_kit_index = -1;
						selected_skin_name = "";
					}

					for (size_t w = 0; w < k_gloves.size(); w++)
					{
						for (auto names : k_gloves[w].weaponName)
						{
							std::string name = k_gloves[w].name;
							//name += " | ";
							//name += names;

							if (g_Options.changers.skin.show_cur)
							{
								if (names != weaponName)
									continue;
							}

							if (name.find(filter) != name.npos)
							{
								ImGui::PushID(lastID++);

								ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(6));
								{
									if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
									{
										selected_entry.paint_kit_vector_index = w;
										selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
										selected_skin_name = k_gloves[selected_entry.paint_kit_vector_index].name_short;
									}
								}
								ImGui::PopStyleColor();

								ImGui::PopID();
							}
						}
					}
				}
			}
			//	ImGui::ListBoxFooter();
		}
		else
		{
			ImGui::Text("skins parsing, wait...");
		}
		//ImGui::Checkbox("skin preview", &g_Options.changers.skin.skin_preview);
		/*ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);
		ImGui::InputInt("seed", &selected_entry.seed);
		ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
		ImGui::SliderFloat("wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
		*/


	}
	ImGui::EndChild();

	ImGui::SetCursorPos({ 41 + 166 + 166,65 });
	ImGui::BeginChild("##3", { 166,276 });
	{			
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("   CT Player Model");
		ImGui::PushItemWidth(160.f);
		ImGui::Combo("##TPlayerModel", &g_Options.playerModelCT, "Default\0Special Agent Ava\0Operator\0Markus Delrow\0Michael Syfers\0B Squadron Officer\0Seal Team 6 Soldier\0Buckshot\0Commander Ricksaw\0Third Commando\0'Two Times' McCoy\0Dragomir\0Rezan The Ready\0The Doctor Romanov\0Maximus\0Blackwolf\0The Elite Muhlik\0Ground Rebel\0Osiris\0 hahmat\0Enforcer\0Slingshot\0Soldier\0");

		ImGui::Text("   T Player Model");
		ImGui::PushItemWidth(160.f);
		ImGui::Combo("##CTPlayerModel", &g_Options.playerModelT, "Default\0Special Agent Ava\0Operator\0Markus Delrow\0Michael Syfers\0B Squadron Officer\0Seal Team 6 Soldier\0Buckshot\0Commander Ricksaw\0Third Commando\0'Two Times' McCoy\0Dragomir\0Rezan The Ready\0The Doctor Romanov\0Maximus\0Blackwolf\0The Elite Muhlik\0Ground Rebel\0Osiris\0Shahmat\0Enforcer\0Slingshot\0Soldier\0");

		if (ImGui::Button(" update skin"))
		{
			static auto clear_hud_weapon_icon_fn =
				reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
					Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC"));

			auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

			if (!element) return;

			auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xa0);
			if (hud_weapons == nullptr) return;

			if (!*hud_weapons->get_weapon_count()) return;

			for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
				i = clear_hud_weapon_icon_fn(hud_weapons, i);

			typedef void(*ForceUpdate) (void);
			static ForceUpdate FullUpdate = (ForceUpdate)Utils::PatternScan(GetModuleHandleA("engine.dll"), "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");
			FullUpdate();
		}

	}
	ImGui::EndChild();
	}
	}
	ImGui::End();
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
/*	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.000f, 0.545f, 1.000f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.060f, 0.416f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	ImGui::GetStyle() = _style;*/

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.900000f, 0.900000f, 0.900000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.600000f, 0.600000f, 0.600000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.000000f, 0.000000f, 0.000000f, 0.4f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.000000f, 0.000000f, 0.000000f, 0.000000f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.110000f, 0.110000f, 0.110000f, 0.920000f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.500000f, 0.500000f, 0.500000f, 0.500000f));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.000000f, 0.000000f, 0.000000f, 0.000000f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.430000f, 0.430000f, 0.430000f, 0.390000f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.470000f, 0.470000f, 0.470000f, 0.400000f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.40000f, 0.40000f, 0.40000f, 0.690000f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.270000f, 0.270000f, 0.540000f, 0.830000f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.320000f, 0.320000f, 0.630000f, 0.870000f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.400000f, 0.400000f, 0.800000f, 0.200000f));
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.400000f, 0.400000f, 0.550000f, 0.800000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.200000f, 0.250000f, 0.300000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.400000f, 0.400000f, 0.800000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.400000f, 0.400000f, 0.800000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.410000f, 0.390000f, 0.800000f, 0.00000f));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.900000f, 0.900000f, 0.900000f, 0.500000f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.000000f, 1.000000f, 1.000000f, 0.300000f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.410000f, 0.390000f, 0.800000f, 0.600000f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.350000f, 0.400000f, 0.610000f, 0.0000f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.400000f, 0.480000f, 0.710000f, 0.0000f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.460000f, 0.540000f, 0.800000f, 0.000000f));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.400000f, 0.400000f, 0.400000f, 0.450000f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.400000f, 0.400000f, 0.400000f, 0.800000f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.400000f, 0.400000f, 0.400000f, 0.800000f));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.500000f, 0.500000f, 0.500000f, 0.600000f));
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.600000f, 0.600000f, 0.700000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.700000f, 0.700000f, 0.900000f, 1.000000f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(1.000000f, 1.000000f, 1.000000f, 0.160000f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0.780000f, 0.820000f, 1.000000f, 0.600000f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.780000f, 0.820000f, 1.000000f, 0.900000f));
	ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.000000f, 0.000000f, 1.000000f, 0.350000f));
	ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(1.000000f, 1.000000f, 0.000000f, 0.900000f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.000000f,6.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.000000f,3.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.000000f,4.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 4.000000f,4.000000f });
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 21.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 0.000000f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.000000f,0.500000f });
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.500000f,0.500000f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.500000f,0.500000f });
}

