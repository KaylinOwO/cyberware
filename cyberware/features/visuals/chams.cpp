#include "chams.hpp"
#include <fstream>

#include "../../valve_sdk/csgostructs.hpp"
#include "../../options.hpp"
#include "../../hooks.hpp"
#include "../../helpers/input.hpp"
#include "../aimbot/backtrack.h"

void modulate(const Color color, IMaterial* material)
{
	if (!g_EngineClient->IsInGame())
		return;

	if (material)
	{
		material->ColorModulate(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f);
		material->AlphaModulate(color[3] / 255.f);
		const auto tint = material->find_var("$envmaptint");

		if (tint)
		{
			tint->set_vector(Vector(color.r(), color.g(), color.b()));
		}
	}

	g_RenderView->SetColorModulation(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f);
}

void Chams::OnDrawModelExecute(void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
{
	static auto fnDME = Hooks::stdrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute2)>(index::DrawModelExecute2);

	if (!pInfo->m_pClientEntity || !g_LocalPlayer)
		return;

	const auto mdl = pInfo->m_pClientEntity->GetModel();
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	static IMaterial* shine = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_OTHER);
	if (is_player) {
		const auto ent = (C_BasePlayer*)pInfo->m_pClientEntity->GetIClientUnknown()->GetBaseEntity();
		if (ent && ent->IsAlive()) {
			if (ent->IsPlayer() && ent->IsEnemy())
			{
				static IMaterial* player_enemies_type = nullptr;
				switch (g_Options.chams_player_flat)
				{
				case 0:
					player_enemies_type = g_MatSystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL);
					break;

				case 1:
					player_enemies_type = g_MatSystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL);
					break;

				case 2:
					player_enemies_type = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_OTHER);
					break;
				}

				static IMaterial* player_backtrack_type = nullptr;
				switch (g_Options.chams_player_backtrack_type)
				{
				case 0:
					player_backtrack_type = g_MatSystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL);
					break;

				case 1:
					player_backtrack_type = g_MatSystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL);
					break;

				case 2:
					player_backtrack_type = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_OTHER);
					break;
				}
				if (player_backtrack_type != nullptr)
				{
					if (g_Options.chams_player_backtrack && CLagCompensation::Get().data.count(ent->EntIndex()))
					{
						const auto records = CLagCompensation::Get().data.at(ent->EntIndex());
						if (!records.empty() && CLagCompensation::Get().ValidTick(records.front().simTime) && !(records.back().Origin == ent->abs_origin()) && !(records.back().boneMatrix == pBoneToWorld)) {
							player_backtrack_type->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
							modulate(g_Options.color_chams_backtrack, player_backtrack_type);
							g_StudioRender->ForcedMaterialOverride(player_backtrack_type);

							if (g_Options.chams_player_backtrack_allticks)
								for (auto t = 1; t < records.size(); t++) { fnDME(g_StudioRender, 0, pResults, pInfo, (matrix3x4_t*)records.at(t).boneMatrix, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags); }
							else
								fnDME(g_StudioRender, 0, pResults, pInfo, (matrix3x4_t*)records.back().boneMatrix, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

							if (!g_Options.chams_player_enabled)
							{
								g_StudioRender->ForcedMaterialOverride(nullptr);
								fnDME(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
							}
						}
					}
				}
				if (player_enemies_type != nullptr && shine != nullptr)
				{
					if (g_Options.chams_player_ignorez && g_Options.chams_player_enabled)
					{
						player_enemies_type->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						modulate(g_Options.color_chams_player_enemy_occluded, player_enemies_type);
						g_StudioRender->ForcedMaterialOverride(player_enemies_type);
						fnDME(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
					}
					if (g_Options.chams_player_enabled)
					{
						player_enemies_type->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						modulate(g_Options.color_chams_player_enemy_visible, player_enemies_type);
						g_StudioRender->ForcedMaterialOverride(player_enemies_type);
						fnDME(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
						if (g_Options.player_enemies_shine)
						{
							shine->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
							shine->AlphaModulate(g_Options.player_enemy_visible_shine[3] / 255.f);
							bool bFound = false;
							auto pVar = shine->FindVar("$envmaptint", &bFound);
							if (bFound)
							{
								(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, g_Options.player_enemy_visible_shine[0] / 255.f, g_Options.player_enemy_visible_shine[1] / 255.f, g_Options.player_enemy_visible_shine[2] / 255.f);
							}
							g_MdlRender->ForcedMaterialOverride(shine);
						}
					}
				}
			}
		}
	}

}