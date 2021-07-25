#include "aimbot.hpp"
#include "autowall.hpp"
#include "../../menu.hpp"
#include "../..//helpers/math.hpp"
#include "../..//helpers/input.hpp"

void CLegitbot::WeaponSettings()
{
	auto pWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!pWeapon)
		return;

	auto pWeaponData = pWeapon->GetCSWeaponData();
	if (!pWeaponData)
		return;

	//I'm almost certain there's a better way to do this 

	switch (pWeaponData->iWeaponType)
	{
		case WEAPONTYPE_PISTOL:
		{
			aim_smooth = g_Options.aim_smooth_pistol;
			aim_fov = g_Options.aim_fov_pistol;
			if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_ELITE || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_CZ75A)
				aim_rcs = g_Options.aim_rcs_pistol;
			else
				aim_rcs = false;
			aim_rcs_type = g_Options.aim_rcs_type_pistol;
			aim_rcs_x = g_Options.aim_rcs_x_pistol;
			aim_rcs_y = g_Options.aim_rcs_y_pistol;
			aim_silent = g_Options.aim_silent_pistol;

			aim_autowall = g_Options.aim_autowall_pistol;
			aim_mindamage = g_Options.aim_mindamage_pistol;

			aim_type = g_Options.aim_type_pistol;
			aim_hitbox = g_Options.aim_hitbox_pistol;


			//Aim_Hitbox = C::Get<int>(Vars.iHitboxPistol);
			break;
		}
		case WEAPONTYPE_RIFLE:
		{
			aim_smooth = g_Options.aim_smooth_rifle;
			aim_fov = g_Options.aim_fov_rifle;
			aim_rcs = g_Options.aim_rcs_rifle;
			aim_rcs_type = g_Options.aim_rcs_type_rifle;
			aim_rcs_x = g_Options.aim_rcs_x_rifle;
			aim_rcs_y = g_Options.aim_rcs_y_rifle;
			aim_silent = g_Options.aim_silent_rifle;

			aim_shotdelay = g_Options.aim_shotdelay_rifle;

			aim_autowall = g_Options.aim_autowall_rifle;
			aim_mindamage = g_Options.aim_mindamage_rifle;

			aim_type = g_Options.aim_type_rifle;
			aim_hitbox = g_Options.aim_hitbox_rifle;

			break;
		}
		case WEAPONTYPE_SHOTGUN:
		{
			aim_smooth = g_Options.aim_smooth_heavy;
			aim_fov = g_Options.aim_fov_heavy;
			aim_rcs = false;
			aim_silent = g_Options.aim_silent_heavy;

			aim_shotdelay = 0;

			aim_autowall = g_Options.aim_autowall_heavy;
			aim_mindamage = g_Options.aim_mindamage_heavy;

			aim_type = g_Options.aim_type_heavy;
			aim_hitbox = g_Options.aim_hitbox_heavy;

			break;
		}
		case WEAPONTYPE_MACHINEGUN:
		{
			aim_smooth = g_Options.aim_smooth_heavy;
			aim_fov = g_Options.aim_fov_heavy;
			aim_rcs = g_Options.aim_rcs_heavy;
			aim_rcs_type = g_Options.aim_rcs_type_heavy;
			aim_rcs_x = g_Options.aim_rcs_x_heavy;
			aim_rcs_y = g_Options.aim_rcs_y_heavy;
			aim_silent = g_Options.aim_silent_heavy;

			aim_shotdelay = g_Options.aim_shotdelay_heavy;

			aim_autowall = g_Options.aim_autowall_heavy;
			aim_mindamage = g_Options.aim_mindamage_heavy;

			aim_type = g_Options.aim_type_heavy;
			aim_hitbox = g_Options.aim_hitbox_heavy;
			break;
		}
		case WEAPONTYPE_SUBMACHINEGUN:
		{
			aim_smooth = g_Options.aim_smooth_smg;
			aim_fov = g_Options.aim_fov_smg;
			aim_rcs = g_Options.aim_rcs_smg;
			aim_rcs_type = g_Options.aim_rcs_type_smg;
			aim_rcs_x = g_Options.aim_rcs_x_smg;
			aim_rcs_y = g_Options.aim_rcs_y_smg;
			aim_silent = g_Options.aim_silent_smg;

			aim_shotdelay = g_Options.aim_shotdelay_smg;

			aim_autowall = g_Options.aim_autowall_smg;
			aim_mindamage = g_Options.aim_mindamage_smg;

			aim_type = g_Options.aim_type_smg;
			aim_hitbox = g_Options.aim_hitbox_smg;

			break;
		}
		case WEAPONTYPE_SNIPER:
		{
			if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08)
			{
				aim_smooth = g_Options.aim_smooth_scout;
				aim_fov = g_Options.aim_fov_scout;
				aim_silent = g_Options.aim_silent_scout;

				aim_shotdelay = 0;

				aim_autowall = g_Options.aim_autowall_scout;
				aim_mindamage = g_Options.aim_mindamage_scout;

				aim_rcs = false;

				aim_type = g_Options.aim_type_scout;
				aim_hitbox = g_Options.aim_hitbox_scout;

			}
			else
			{
				aim_smooth = g_Options.aim_smooth_sniper;
				aim_fov = g_Options.aim_fov_sniper;
				if (pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)
					aim_rcs = false;
				else
					aim_rcs = g_Options.aim_rcs_sniper;
				aim_rcs_type = g_Options.aim_rcs_type_sniper;
				aim_rcs_x = g_Options.aim_rcs_x_sniper;
				aim_rcs_y = g_Options.aim_rcs_y_sniper;
				aim_silent = g_Options.aim_silent_sniper;

				aim_shotdelay = 0;

				aim_autowall = g_Options.aim_autowall_sniper;
				aim_mindamage = g_Options.aim_mindamage_sniper;

				aim_type = g_Options.aim_type_sniper;
				aim_hitbox = g_Options.aim_hitbox_sniper;
			}
			break;
		}

	}
}

bool CLegitbot::IsRcs() {
	return g_LocalPlayer->m_iShotsFired() >= aim_rcs_start + 1;
}

float CLegitbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}

bool CLegitbot::IsLineGoesThroughSmoke(Vector startPos, Vector endPos)
{
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector, Vector))Utils::PatternScan2("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(startPos, endPos);
}

bool CLegitbot::IsEnabled(CUserCmd* cmd)
{
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer)
		return false;

	if (!g_LocalPlayer->IsAlive())
		return false;

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsGun())
		return false;

	if (!g_Options.aim_enabled)
		return false;

	if (!weapon->HasBullets() || weapon->IsReloading())
		return false;

	if (Menu::Get().IsVisible())
		return false;

	return (cmd->buttons & IN_ATTACK);
}


void CLegitbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	auto smooth_value = aim_smooth;
	if (smooth_value <= 1) return;

	Vector current, aim;

	QAngle delta = aimAngle - currentAngle;
	Math::FixAngles(delta);

	smooth_value = (g_GlobalVars->interval_per_tick * 64.0f) / smooth_value;

	delta *= smooth_value;
	angle = currentAngle + delta;
	Math::FixAngles(angle);
}

QAngle CurrentPunch = { 0,0,0 };
QAngle RCSLastPunch = { 0,0,0 };
void CLegitbot::RCS(QAngle& angle, C_BasePlayer* target, bool should_run)
{
	if (!aim_rcs) {
		RCSLastPunch.Init();
		return;
	}

	if (aim_rcs_x == 0 && aim_rcs_y == 0) {
		RCSLastPunch.Init();
		return;
	}

	QAngle punch = g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (weapon && weapon->m_flNextPrimaryAttack() > g_GlobalVars->curtime) {
		auto delta_angles = punch - RCSLastPunch;
		auto delta = weapon->m_flNextPrimaryAttack() - g_GlobalVars->curtime;
		if (delta >= g_GlobalVars->interval_per_tick)
			punch = RCSLastPunch + delta_angles / static_cast<float>(TIME_TO_TICKS(delta));
	}

	CurrentPunch = punch;
	if (aim_rcs_type == 0 && !should_run)
		punch -= { RCSLastPunch.pitch, RCSLastPunch.yaw, 0.f };

	RCSLastPunch = CurrentPunch;
	if (!IsRcs()) return;

	float pithcmult = aim_rcs_x;

	if (g_LocalPlayer->m_iShotsFired() < 4 + aim_rcs_start) pithcmult += 10;
	punch.pitch *= (pithcmult / 100.0f);
	punch.yaw *= (aim_rcs_y / 100.0f);

	angle -= punch;
	Math::FixAngles(angle);
}

bool CLegitbot::IsSilent()
{
	return aim_silent;
}

float CLegitbot::GetFov()
{
	return aim_fov;
}

bool CLegitbot::IsWallbangable(C_BasePlayer* pEntity, CUserCmd* pCmd, Vector Hitbox)
{
	float damage = Autowall::GetDamage(Hitbox);
	return (aim_autowall ? damage > (aim_mindamage == 101 ? pEntity->m_iHealth() + 1 : aim_mindamage) : g_LocalPlayer->IsVisible(pEntity, g_LocalPlayer->GetEyePos(), Hitbox));
}

int CLegitbot::GetBestHitbox(C_BasePlayer* entity, CUserCmd* cmd)
{
	if (!entity || entity->IsNotTarget()) return -1;

	int Hitbox = aim_hitbox;  

	for (int i = Hitbox; i < entity->GetHitboxNum(); i++)
		if (IsWallbangable(entity, cmd, entity->GetHitboxPos(i)))
			return i;

	if (!IsWallbangable(entity, cmd, entity->GetHitboxPos(Hitbox)))
		return -1;

	return Hitbox;
}

C_BasePlayer* CLegitbot::GetClosestPlayer(CUserCmd* cmd, int& bestBone)
{
	QAngle ang;
	Vector eVecTarget;
	Vector pVecTarget = g_LocalPlayer->GetEyePos();
	if (target && target->IsNotTarget()) {
		target = NULL;
		shot_delay = false;
	}

	C_BasePlayer* player;
	target = NULL;
	int bestHealth = 100.f;
	float bestFov = 9999.f;
	float bestDamage = 0.f;
	float bestBoneFov = 9999.f;
	float bestDistance = 9999.f;
	int health;
	float fov;
	float damage;
	float distance;
	int fromBone = aim_type == 1 ? 0 : aim_hitbox;
	int toBone = aim_type == 1 ? 7 : aim_hitbox;
	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i) {
		damage = 0.f;
		player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) continue;

		//if (!aim_deathmatch && player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) continue;

		for (int bone = fromBone; bone <= toBone; bone++) {
			eVecTarget = player->GetHitboxPos(bone);
			Math::VectorAngles(eVecTarget - pVecTarget, ang);
			Math::FixAngles(ang);
			distance = pVecTarget.DistTo(eVecTarget);
			fov = GetFovToPlayer(cmd->viewangles + RCSLastPunch, ang);

			if (fov > GetFov()) continue;

			if (!g_LocalPlayer->CanSeePlayer(player, eVecTarget)) {
				if (!aim_autowall) continue;

				damage = Autowall::GetDamage(eVecTarget);
				if (damage < aim_mindamage) continue;
			}
			if (damage == 0.f)
				damage = Autowall::GetDamage(eVecTarget);

			health = player->m_iHealth() - damage;
			if (g_Options.aim_smoke_check && IsLineGoesThroughSmoke(pVecTarget, eVecTarget)) continue;

		//	bool OnGround = (g_LocalPlayer->m_fFlags() & FL_ONGROUND);
			//if (g_Options.aim_flash_check && !OnGround) continue;

			if (aim_type == 1 && bestBoneFov < fov) continue;

			bestBoneFov = fov;
			if (bestFov > fov) 
			{
				bestBone = bone;
				//if (aim_type == 1)
					//bestBone = GetBestHitbox(player, cmd);
				target = player;
				bestFov = fov;
				bestHealth = health;
				bestDamage = damage;
				bestDistance = distance;
			}
		}
	}
	return target;
}

void CLegitbot::Run(CUserCmd* cmd)
{
	if (!IsEnabled(cmd))
	{
		if (g_LocalPlayer && g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive() && settings.enabled && aim_rcs_type == 0) {
			auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
			if (pWeapon && (pWeapon->IsSniper() || pWeapon->IsPistol() || pWeapon->IsRifle())) {
				RCS(cmd->viewangles, target, false);
				Math::FixAngles(cmd->viewangles);
				g_EngineClient->SetViewAngles(&cmd->viewangles);
			}
		}
		else RCSLastPunch = { 0, 0, 0 };

		last_punch = { 0, 0, 0 };
		shot_delay = false;
		kill_delay = false;
		target = nullptr;
		return;
	}

	WeaponSettings();

	current_punch = g_LocalPlayer->m_aimPunchAngle();

	//RandomSeed(cmd->command_number);

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;

	bool should_do_rcs = false;

	if (g_Options.aim_flash_check && g_LocalPlayer->IsFlashed())
		return;

	QAngle angles = cmd->viewangles;
	QAngle current = angles;
	float fov = 180.f;
	if (!(settings.flash_check && g_LocalPlayer->IsFlashed())) {
		int bestBone = -1, hitscanBone = -1;
		if (GetClosestPlayer(cmd, bestBone)) {
			hitscanBone = GetBestHitbox(GetClosestPlayer(cmd, bestBone), cmd);
			if (bestBone == -1) return;
			Math::VectorAngles(target->GetHitboxPos(bestBone) - g_LocalPlayer->GetEyePos(), angles);
			Math::FixAngles(angles);
			fov = GetFovToPlayer(cmd->viewangles, angles);

			should_do_rcs = true;
			g_AimbotTargetIndex = target->EntIndex();
		}
		else
			g_AimbotTargetIndex = 0;
	}

	if (!aim_silent && (should_do_rcs || aim_rcs_type == 0)) RCS(angles, target, should_do_rcs);

	if (target && !aim_silent) Smooth(current, angles, angles);

	Math::FixAngles(angles);
	cmd->viewangles = aim_silent ? angles - (g_LocalPlayer->m_aimPunchAngle() * 2.0f) : angles;
	if (!aim_silent)
		g_EngineClient->SetViewAngles(&angles);

	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol() && settings.autopistol) {
		float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;
		if (next_shot > 0) cmd->buttons &= ~IN_ATTACK;
	}
}