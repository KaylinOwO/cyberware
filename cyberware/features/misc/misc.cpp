#include "misc.h"
#include "../../options.hpp"
#include "../../helpers/math.hpp"

void CMisc::Desync(CUserCmd* pCmd, bool* bSendPacket)
{
	if (!g_Options.Desync || pCmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_USE) || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || !g_LocalPlayer->IsAlive())
		return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon) return;

	auto weapon_index = weapon->m_Item().m_iItemDefinitionIndex();
	if ((weapon_index == WEAPON_GLOCK || weapon_index == WEAPON_FAMAS) && weapon->m_flNextPrimaryAttack() >= g_GlobalVars->curtime)
		return;

	auto weapon_data = weapon->GetCSWeaponData();

	if (weapon_data->iWeaponType == WEAPONTYPE_GRENADE) {
		if (!weapon->m_bPinPulled()) {
			float throwTime = weapon->m_fThrowTime();
			if (throwTime > 0.f)
				return;
		}

		if ((pCmd->buttons & IN_ATTACK) || (pCmd->buttons & IN_ATTACK2)) {
			if (weapon->m_fThrowTime() > 0.f)
				return;
		}
	}

	static bool broke_lby = false;
	QAngle OldAngles = pCmd->viewangles;

	if (GetAsyncKeyState(g_Options.DesyncSwitchKey) & 1)
		side = -side;

	if (next_lby >= g_GlobalVars->curtime) {
		if (!broke_lby && *bSendPacket && g_ClientState->m_nChokedCommands > 0)
			return;

		broke_lby = false;
		*bSendPacket = false;
		pCmd->viewangles.yaw += 120.0f * side;
	}
	else {
		broke_lby = true;
		*bSendPacket = false;
		pCmd->viewangles.yaw += 120.0f * -side;
	}

	/*
			float minimal_move = g_LocalPlayer->m_fFlags() & IN_DUCK ? 3.0f : 1.0f;
		bool should_move = g_LocalPlayer->m_vecVelocity().Length2D() <= 0.0f ||
			std::fabsf(g_LocalPlayer->m_vecVelocity().z) <= 100.0f;

			if (pCmd->command_number % 2 == 1) {
				pCmd->viewangles.yaw += 120.0f * side;
				if (should_move)
					pCmd->sidemove -= minimal_move;
				*bSendPacket = false;
			}
			else if (should_move)
				pCmd->sidemove += minimal_move;*/

}

void CMisc::CorrectMouse(CUserCmd* cmd) {
	static ConVar* m_yaw = m_yaw = g_CVar->FindVar("m_yaw");
	static ConVar* m_pitch = m_pitch = g_CVar->FindVar("m_pitch");
	static ConVar* sensitivity = sensitivity = g_CVar->FindVar("sensitivity");
	static QAngle m_angOldViewangles = g_ClientState->viewangles;

	float delta_x = std::remainderf(cmd->viewangles.pitch - m_angOldViewangles.pitch, 360.0f);
	float delta_y = std::remainderf(cmd->viewangles.yaw - m_angOldViewangles.yaw, 360.0f);

	if (delta_x != 0.0f) {
		float mouse_y = -((delta_x / m_pitch->GetFloat()) / sensitivity->GetFloat());
		short mousedy;
		if (mouse_y <= 32767.0f) {
			if (mouse_y >= -32768.0f) {
				if (mouse_y >= 1.0f || mouse_y < 0.0f) {
					if (mouse_y <= -1.0f || mouse_y > 0.0f)
						mousedy = static_cast<short>(mouse_y);
					else mousedy = -1;
				}
				else mousedy = 1;
			}
			else mousedy = 0x8000u;
		}
		else mousedy = 0x7FFF;
		cmd->mousedy = mousedy;
	}
	if (delta_y != 0.0f) {
		float mouse_x = -((delta_y / m_yaw->GetFloat()) / sensitivity->GetFloat());
		short mousedx;
		if (mouse_x <= 32767.0f) {
			if (mouse_x >= -32768.0f) {
				if (mouse_x >= 1.0f || mouse_x < 0.0f) {
					if (mouse_x <= -1.0f || mouse_x > 0.0f)
						mousedx = static_cast<short>(mouse_x);
					else mousedx = -1;
				}
				else mousedx = 1;
			}
			else mousedx = 0x8000u;
		}
		else mousedx = 0x7FFF;
		cmd->mousedx = mousedx;
	}
}

QAngle vect2d(QAngle p1, QAngle p2) {
	QAngle temp;
	temp.pitch = (p2.pitch - p1.pitch);
	temp.yaw = -1 * (p2.yaw - p1.yaw);

	return temp;
}
bool pointInRectangle(QAngle A, QAngle B, QAngle C, QAngle D, QAngle m) {
	QAngle AB = vect2d(A, B);  float C1 = -1 * (AB.yaw * A.pitch + AB.pitch * A.yaw); float  D1 = (AB.yaw * m.pitch + AB.pitch * m.yaw) + C1;
	QAngle AD = vect2d(A, D);  float C2 = -1 * (AD.yaw * A.pitch + AD.pitch * A.yaw); float D2 = (AD.yaw * m.pitch + AD.pitch * m.yaw) + C2;
	QAngle BC = vect2d(B, C);  float C3 = -1 * (BC.yaw * B.pitch + BC.pitch * B.yaw); float D3 = (BC.yaw * m.pitch + BC.pitch * m.yaw) + C3;
	QAngle CD = vect2d(C, D);  float C4 = -1 * (CD.yaw * C.pitch + CD.pitch * C.yaw); float D4 = (CD.yaw * m.pitch + CD.pitch * m.yaw) + C4;
	return     0 >= D1 && 0 >= D4 && 0 <= D2 && 0 >= D3;
}

bool Checkifbetween(QAngle* ViewanglePoints, QAngle Viewangles)
{
	if (pointInRectangle(ViewanglePoints[0], ViewanglePoints[1], ViewanglePoints[2], ViewanglePoints[3], Viewangles) //first 4 normal
		|| pointInRectangle(ViewanglePoints[3], ViewanglePoints[2], ViewanglePoints[1], ViewanglePoints[0], Viewangles) //first 4 reverse
		|| pointInRectangle(ViewanglePoints[4], ViewanglePoints[5], ViewanglePoints[6], ViewanglePoints[7], Viewangles) //second 4 normal
		|| pointInRectangle(ViewanglePoints[7], ViewanglePoints[6], ViewanglePoints[5], ViewanglePoints[4], Viewangles))//second 4 reverse
	{
		return true;
	}
	return false;
}


bool WEarechoking = false;

void CMisc::AntiTriggerchoke(bool* fakestuff, bool* oldsendpacket)
{//this one is prolly utter shit xD
	if (!WEarechoking)
		return;

	static int _pchoked = 0;
	if (!_pchoked && !oldsendpacket)
		_pchoked++;

	if (_pchoked <= 13) {
		*fakestuff = false;
		_pchoked++;
	}
	else {
		static int repeats = 0;
		if (repeats >= 1)
			WEarechoking = false;
		repeats++;
		*fakestuff = true;
		_pchoked = 0;
	}
}

void CMisc::AntiTrigger(CUserCmd* cmd)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->m_vecVelocity().Length2D() < 100 || WEarechoking) // skip function if we are dead or not really moving
		return;

	for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++) // loop though maxent
	{
		C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!pEntity) return;
		if (pEntity->IsAlive() && !pEntity->IsDormant() && pEntity->m_vecVelocity().Length2D() < 50) {// check if not teammate, and all that shit (could need a weaponcheck)
			QAngle ViewAngles = pEntity->m_angEyeAngles(); // get their viewangles
			Vector fowardVec; //placeholder

			Math::AngleVectors(ViewAngles, fowardVec);
			fowardVec *= 8012.f;

			Vector
				start = pEntity->GetEyePos(),
				end = start + fowardVec;

			auto
				org = g_LocalPlayer->abs_origin(), // this prolly needs adjusting, extrapolt urself by x ticks
				min = g_LocalPlayer->GetCollideable()->OBBMins(),
				max = g_LocalPlayer->GetCollideable()->OBBMaxs();
			Vector boxaroundme[8] = { //the comments below are not accurate
				Vector(org.x + min.x, org.y + min.y, org.z + min.z), // left buttom
				Vector(org.x + min.x, org.y + min.y, org.z + max.z), // left top
				Vector(org.x + max.x, org.y + max.y, org.z + max.z), // right top
				Vector(org.x + max.x, org.y + max.y, org.z + min.z),  // right buttom

				Vector(org.x + min.x, org.y + max.y, org.z + min.z), // left buttom
				Vector(org.x + min.x, org.y + max.y, org.z + max.z), // left top
				Vector(org.x + max.x, org.y + min.y, org.z + max.z), // right top
				Vector(org.x + max.x, org.y + min.y, org.z + min.z)  // right buttom
			};
			QAngle Angles[8];
			for (int j = 0; j < 8; j++) { //get angle to each rectangle corner
				Math::CalcAngle2(start, boxaroundme[j], Angles[j]);
			}

			if (!Checkifbetween(Angles, ViewAngles))continue; //skip entity if viewangles outside of rect

			CGameTrace tr;
			Ray_t ray;
			CTraceFilter filter;
			filter.pSkip = pEntity;

			ray.Init(start, end);
			g_EngineTrace->TraceRay(ray, 0x46004003, &filter, &tr);//normal raycasting, u might wanna change the mask so the ray doesnt collide with hitboxes

			if ((tr.endpos - start).Length() + 10 >= (g_LocalPlayer->GetEyePos() - start).Length()) { //pretty ghetto intersect check
				WEarechoking = true; // globalvar for should choke
				break;
			}
		}
	}
}