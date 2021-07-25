#include <algorithm>

#include "visuals.hpp"
#include "../aimbot/backtrack.h"

#include "../../options.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"
#include "grenade_prediction.h"

bool lastvelsaved = false; //saver 
int lastjump, lastvel, lasttick = 0; // last vel holder 
std::string drawvel; //text drawer holder 
std::string drawvel2;

Color HealthColor(C_BasePlayer* e)
{
	int iMaxHealth, Health;
	iMaxHealth = 100;
	Health = e->m_iHealth();
	auto iHalfHealth = iMaxHealth / 2;
	auto iThirdOfHealth = iHalfHealth / 2;


	if (Health > iHalfHealth && Health <= iMaxHealth)
		return Color(0, 255, 0, 255);
	else if (Health > iThirdOfHealth && Health <= iHalfHealth)
		return Color(255, 255, 0, 255);
	else if (Health > 0 && Health <= iThirdOfHealth)
		return Color(255, 0, 0, 255);

	return Color(0, 255, 0, 255);
}


Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Visuals::Render() {
	PlayerESP();
//	RenderSounds();
	GrenadePrediction::draw();
	Hitmarker();
	Velocity();
}

void Visuals::ESP(C_BasePlayer* e, SBoundingBox Box, bool EntityIsAlive)
{
	if (e->IsDormant() || EntityIsAlive && !e->IsAlive()) return;

	if (g_Options.esp_player_boxes)
	{
		gDrawManager.OutlineRect(Box.x - 1, Box.y - 1, Box.w + 2, Box.h + 2, Color(0, 0, 0, 255));
		gDrawManager.OutlineRect(Box.x, Box.y, Box.w, Box.h, g_Options.color_esp_enemy);
		gDrawManager.OutlineRect(Box.x + 1, Box.y + 1, Box.w - 2, Box.h - 2, Color(0, 0, 0, 255));
	}
	if (EntityIsAlive && g_Options.esp_player_health)
	{
		int iY = 0;
		int Health = Health = e->m_iHealth();
		int UnclampedHealth = e->m_iHealth();
		int MaxHealth = 100;

		Health = std::clamp(Health, 0, MaxHealth);
		const auto HealthBarHeight = (Health * (Box.h + 1)) / MaxHealth;

		if (UnclampedHealth < MaxHealth || UnclampedHealth > MaxHealth)
			gDrawManager.DrawString(Box.x - (14 / 2), Box.y + 2 + ((Box.h) - HealthBarHeight) - 7, Color(255, 255, 255, 255), Font::Get().Name, std::to_string(UnclampedHealth).c_str(), TextAlignment::AlignmentRight);

		gDrawManager.OutlineRect(Box.x - 6, Box.y, 2, Box.h, Color(0, 0, 0, 200));
		gDrawManager.OutlineRect(Box.x - 6, Box.y + ((Box.h) - HealthBarHeight), 2, HealthBarHeight, HealthColor(e));
		gDrawManager.OutlineRect(Box.x - 6 - 1, Box.y - 1, 2 + 2, Box.h + 2, Color(0, 0, 0, 225));

		if (UnclampedHealth > MaxHealth) {
			UnclampedHealth = MaxHealth;
		}
	}
}
void Visuals::PlayerESP()
{
	if (!g_LocalPlayer || g_LocalPlayer->IsDormant())
		return;

	for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++)
	{
		if (i == g_LocalPlayer->EntIndex())
			continue;

		C_BasePlayer* pEntity = (C_BasePlayer*)C_BasePlayer::GetEntityByIndex(i);
		player_info_t pInfo;
		SBoundingBox Box;

		if (!pEntity || pEntity->IsDormant() || !pEntity->IsAlive() || !pEntity->IsEnemy() || !g_Options.esp_enabled || g_Options.esp_visonly && !pEntity->VisibleTo(g_LocalPlayer))
			continue;

		if (!g_EngineClient->GetPlayerInfo(pEntity->EntIndex(), &pInfo))
			return;

		int iY = 0;
		if (GetBoundingBoxPositions(pEntity, Box)) {
			ESP(pEntity, Box);
			if (g_Options.esp_player_names)
				gDrawManager.DrawString(Box.x + (Box.w / 2), Box.y - 14, Color(255, 255, 255, 255), Font::Get().Name, pInfo.szName, TextAlignment::AlignmentCenter);

			if (g_Options.esp_player_weapons)
			{
				auto weapon = pEntity->m_hActiveWeapon().Get();
				if (!weapon) return;
				if (!weapon->GetCSWeaponData()) return;
				gDrawManager.DrawString(Box.x + (Box.w / 2), Box.y + Box.h + 5, Color(255, 255, 255, 255), Font::Get().Name, weapon->GetCSWeaponData()->szWeaponName, TextAlignment::AlignmentCenter);
			}

			
			/*if (g_Options.misc_bt)
			{
				if (!CLagCompensation::Get().data.count(pEntity->EntIndex())) return;
				const auto records = CLagCompensation::Get().data.at(pEntity->EntIndex());
				if (records.empty() || !CLagCompensation::Get().ValidTick(records.front().simTime)) return;
				for (int j = 0; j < pEntity->GetHitboxNum(); j++)
				{
					const auto pModel = pEntity->GetModel();
					if (!pModel)
						continue;

					studiohdr_t* sm = (studiohdr_t*)g_MdlInfo->GetStudiomodel(pModel);
					if (!sm)
						continue;

					mstudiohitboxset_t* set = sm->GetHitboxSet(pEntity->m_nHitboxSet());
					if (!set)
						continue;

					mstudiobone_t* pBone = sm->GetBone(j);
					if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
					{
						Vector vChild = pEntity->GetBtBonePos(j);
						Vector vParent = pEntity->GetBtBonePos(pBone->parent);

						int iChestBone = 6;  // Parameter of relevant Bone number
						Vector vBreastBone; // New reference Point for connecting many bones
						Vector vUpperDirection = pEntity->GetBtBonePos(iChestBone + 1) - pEntity->GetBtBonePos(iChestBone); // direction vector from chest to neck
						vBreastBone = pEntity->GetBtBonePos(iChestBone) + vUpperDirection / 2;
						Vector vDeltaChild = vChild - vBreastBone; // Used to determine close bones to the reference point
						Vector vDeltaParent = vParent - vBreastBone;

						// Eliminating / Converting all disturbing bone positions in three steps.
						if ((vDeltaParent.Length2D() < 9 && vDeltaChild.Length2D() < 9))
							vParent = vBreastBone;

						if (j == iChestBone - 1)
							vChild = vBreastBone;

						if (abs(vDeltaChild.z) < 5 && (vDeltaParent.Length2D() < 5 && vDeltaChild.Length2D() < 5) || j == iChestBone)
							continue;

						Vector sParent, sChild;
						g_DebugOverlay->ScreenPosition(vParent, sParent);
						g_DebugOverlay->ScreenPosition(vChild, sChild);

						g_VGuiSurface->DrawSetColor(255, 255, 255, 255);
						g_VGuiSurface->DrawLine(sParent[0], sParent[1], sChild[0], sChild[1]);
					}
				}
			}*/
			
		}
	}
}
#undef min
void Visuals::Hitmarker()
{
	Color colLines = Color(g_Options.color_hitmarker.r(), g_Options.color_hitmarker.g(), g_Options.color_hitmarker.b(), 255);
	Color colDamage = Color(g_Options.color_damageindicator.r(), g_Options.color_damageindicator.g(), g_Options.color_damageindicator.b(), 255);
	if (!g_LocalPlayer || g_LocalPlayer->IsDormant() || !g_LocalPlayer->IsAlive() || vecHitMarks.empty())
		return;

	float flServerTime = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase());
	const float flLastDelta = vecHitMarks.back().flTime - flServerTime;

	if (flLastDelta <= 0.f)
		return;

	const float flMaxLinesAlpha = colLines.a();
	constexpr std::array<std::array<float, 2U>, 4U> arrSides = { { { -1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 1.0f }, { 1.0f, -1.0f } } };


	for (const auto& arrSide : arrSides)
	{
		// set fade out alpha
		colLines[3] = static_cast<std::uint8_t>(std::min(flMaxLinesAlpha, flLastDelta / 1.0f) * 255.f);
		// draw mark cross
		if (g_Options.hitmarker)
			gDrawManager.DrawLine(g_ScreenSizeW * 0.5f + 5 * arrSide[0], g_ScreenSizeH * 0.5f + 5 * arrSide[1], g_ScreenSizeW * 0.5f + 10 * arrSide[0], g_ScreenSizeH * 0.5f + 10 * arrSide[1], colLines);
	}

	const float flMaxDamageAlpha = colDamage.a();
	for (std::size_t i = 0U; i < vecHitMarks.size(); i++)
	{
		const float flDelta = vecHitMarks.at(i).flTime - flServerTime;

		if (flDelta <= 0.f)
		{
			vecHitMarks.erase(vecHitMarks.cbegin() + i);
			continue;
		}

		Vector vecScreen = { };
		if (gDrawManager.WorldToScreen(vecHitMarks.at(i).vecPosition, vecScreen))
		{
			// max distance for floating damage
			constexpr float flDistance = 40.f;
			const float flRatio = 1.0f - (flDelta / 1.0f);

			// set fade out alpha
			const int iAlpha = static_cast<int>(std::min(flMaxDamageAlpha, flDelta / 1.0f) * 255.f);
			colDamage[3] = static_cast<std::uint8_t>(iAlpha);

			// draw dealt damage
			if (g_Options.damage_indicator && vecHitMarks.at(i).iDamage > 0)
				gDrawManager.DrawString(vecScreen.x, vecScreen.y - flRatio * flDistance, colDamage, Font::Get().DamageIndicator, std::to_string(vecHitMarks.at(i).iDamage).c_str());
		}
	}

}
void Visuals::Velocity()
{
	if (!g_Options.Velocity || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;
	if (!g_EngineClient->IsInGame())
		return;

	static std::vector<float> velData(120, 0);

	Vector vecVelocity = g_LocalPlayer->m_vecVelocity();
	float currentVelocity = sqrt(vecVelocity.x * vecVelocity.x + vecVelocity.y * vecVelocity.y);

	velData.erase(velData.begin());
	velData.push_back(currentVelocity);

	Vector speed = g_LocalPlayer->m_vecVelocity();
	int intspeed = round(speed.Length2D());
	const float delta = intspeed - lastvel;
	std::string velocity = std::to_string(intspeed);
	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
	{
		if (lastvelsaved)
			lastvelsaved = false;
		drawvel = velocity;
	}
	else
	{
		if (!lastvelsaved)
		{
			lastjump = intspeed;
			lastvelsaved = true;
		}
		if (g_Options.LastJump && lastjump >= 100) drawvel = velocity + " (" + std::to_string(lastjump) + ")"; else drawvel = velocity;
	}

	gDrawManager.DrawString(g_ScreenSizeW / 2, g_ScreenSizeH / 2 + 400, g_Options.Velocitycol, Font::Get().Velocity, drawvel.c_str(), TextAlignment::AlignmentCenter);

	if (g_Options.Graph)
	{
		for (auto i = 0; i < velData.size() - 1; i++)
		{
			int cur = velData.at(i);
			int next = velData.at(i + 1);

			gDrawManager.DrawLine(g_ScreenSizeW / 2 + (velData.size() * 5 / 2) - (i - 1) * 5.f, g_ScreenSizeH / 2 - (std::clamp(cur, 0, 450) * .2f) + 400, g_ScreenSizeW / 2 + (velData.size() * 5 / 2) - i * 5.f, g_ScreenSizeH / 2 - (std::clamp(next, 0, 450) * .2f) + 400, g_Options.Velocitycol);
		}
	}
}
std::map< int, std::vector< SoundInfo_t > > m_Sounds;

void DrawOriginCircle(Vector origin, int radius, Color col = Color(0, 0, 0, 255))
{
	Vector screen;

	if (!gDrawManager.WorldToScreen(origin, screen))
		return;

	gDrawManager.OutlineRect(screen.x, screen.y, 3, 8, col);
}

void Visuals::UpdateSounds() {
	sounds.RemoveAll();
	g_EngineSound->GetActiveSounds(sounds);
	if (sounds.Count() < 1) return;

	Vector eye_pos = g_LocalPlayer->GetEyePos();
	for (int i = 0; i < sounds.Count(); ++i) {
		SndInfo_t& sound = sounds.Element(i);
		if (sound.m_nSoundSource < 1) continue;

		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(sound.m_nSoundSource);
		if (!player) continue;

		if (player->m_hOwnerEntity().IsValid() && player->IsWeapon())
			player = (C_BasePlayer*)player->m_hOwnerEntity().Get();

		if (!player->IsPlayer() || !player->IsAlive()) continue;

		if ((C_BasePlayer*)g_LocalPlayer == player || (g_LocalPlayer->m_iTeamNum() == player->m_iTeamNum())) continue;

		if (player->m_vecOrigin().DistTo(g_LocalPlayer->m_vecOrigin()) > 900) continue;

		auto& player_sound = m_Sounds[player->EntIndex()];
		if (player_sound.size() > 0) {
			bool should_break = false;
			for (const auto& snd : player_sound) {
				if (snd.guid == sound.m_nGuid) {
					should_break = true;
					break;
				}
			}

			if (should_break) continue;
		}

		SoundInfo_t& snd = player_sound.emplace_back();
		snd.guid = sound.m_nGuid;
		snd.soundPos = *sound.m_pOrigin;
		snd.soundTime = g_GlobalVars->realtime;
		snd.alpha = 1.0f;
	}
}

void Visuals::RenderSounds() {

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer) return;
	if (g_LocalPlayer->IsDormant()) return;

	UpdateSounds();

	for (auto& [entIndex, sound] : m_Sounds) {
		if (sound.empty()) continue;

		for (auto& info : sound) {
			if (info.soundTime + 0.5f < g_GlobalVars->realtime)
				info.alpha -= g_GlobalVars->frametime;

			if (info.alpha <= 0.0f) continue;

			float deltaTime = g_GlobalVars->realtime - info.soundTime;

			auto factor = deltaTime / 0.5f;
			if (factor > 1.0f) factor = 1.0f;

			float radius = 15.0f * factor;
			float color_esp_sounds[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			Color color = Color(color_esp_sounds[0], color_esp_sounds[1], color_esp_sounds[2], info.alpha);

			BeamInfo_t beam_info;
			beam_info.m_nType = 7; // 7 for all yall btw
			beam_info.m_pszModelName = "materials/sprites/laserbeam.vmt";
			beam_info.m_nModelIndex = g_MdlInfo->GetModelIndex("materials/sprites/laserbeam.vmt");
			beam_info.m_flHaloScale = 0.0f;
			beam_info.m_flLife = 0.75f;
			beam_info.m_flWidth = 1.5f;
			beam_info.m_flEndWidth = 1.5f;
			beam_info.m_flFadeLength = 1.0f;
			beam_info.m_flAmplitude = 0.f;
			beam_info.m_flBrightness = info.alpha;
			beam_info.m_flSpeed = 2.f;
			beam_info.m_nStartFrame = 0;
			beam_info.m_flFrameRate = 60;
			beam_info.m_flRed = color_esp_sounds[0];
			beam_info.m_flGreen = color_esp_sounds[1];
			beam_info.m_flBlue = color_esp_sounds[2];
			beam_info.m_nSegments = 1;
			beam_info.m_bRenderable = true;
			beam_info.m_nFlags = 0;
			beam_info.m_vecCenter = info.soundPos; //sound location
			beam_info.m_flStartRadius = 0.f;
			beam_info.m_flEndRadius = radius;

			auto beam = g_RenderBeams->CreateBeamRingPoint(beam_info);
			if (beam)
				g_RenderBeams->DrawBeam(beam);
		}

		while (!sound.empty()) {
			auto& back = sound.back();
			if (back.alpha <= 0.0f) sound.pop_back();
			else break;
		}
	}
}


void Visuals::AddToDrawList() {
}
