#pragma once

#include "../..//options.hpp"
#include "../..//valve_sdk/csgostructs.hpp"

class CLegitbot
{
public:
	CLegitbot()
	{
		current_punch = last_punch = { 0, 0, 0 };
		target = nullptr;

		lastShotTick = shotsFired = 0;
		
		shot_delay_time = kill_delay_time = 0;
		shot_delay = kill_delay = false;
	}

	void Run(CUserCmd* cmd);
	bool IsEnabled(CUserCmd* cmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);

	legitbot_s settings;

	float GetFov();

private:
	void WeaponSettings();
	bool IsRcs();
	void RCS(QAngle& angle, C_BasePlayer* target, bool should_run);
	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
	float GetSmooth();
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool IsSilent();
	bool IsWallbangable(C_BasePlayer* pEntity, CUserCmd* pCmd, Vector Hitbox);
	int GetBestHitbox(C_BasePlayer* entity, CUserCmd* cmd);
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int& bestBone);

	C_BasePlayer* target = nullptr;

	int lastShotTick;
	int shotsFired;

	bool shot_delay = false;
	int shot_delay_time = 0;

	bool kill_delay = false;
	int kill_delay_time = 0;

	QAngle current_punch = { 0, 0, 0 };
	QAngle last_punch = { 0, 0, 0 };

	bool aim_silent;
	float aim_fov, aim_smooth;

	int aim_shotdelay;

	int aim_type, aim_hitbox;

	bool aim_rcs;
	int aim_rcs_start, aim_rcs_type, aim_rcs_x, aim_rcs_y;

	bool aim_autowall;
	int aim_mindamage;
};

inline CLegitbot* g_Legitbot;