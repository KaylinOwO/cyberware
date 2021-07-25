#include "bhop.hpp"
#include "../../options.hpp"

#include "../../valve_sdk/csgostructs.hpp"
#include "../../helpers/math.hpp"
#include <filesystem>

void BunnyHop::OnCreateMove(CUserCmd* cmd)
{
  static bool jumped_last_tick = false;
  static bool should_fake_jump = false;
  if (!g_Options.misc_bhop || !g_Options.misc_bhop2)
	  return;
  if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
	  return;

  if(!jumped_last_tick && should_fake_jump) {
    should_fake_jump = false;
    cmd->buttons |= IN_JUMP;
  } else if(cmd->buttons & IN_JUMP) {
    if(g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
      jumped_last_tick = true;
      should_fake_jump = true;
    } else {
      cmd->buttons &= ~IN_JUMP;
      jumped_last_tick = false;
    }
  } else {
    jumped_last_tick = false;
    should_fake_jump = false;
  }
}

void BunnyHop::AutoStafe(CUserCmd* cmd)
{
	if (!g_Options.autostrafe || g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	if (g_Options.AutoStafe_key && !GetAsyncKeyState(g_Options.AutoStafe_key))
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER) return;

	static auto side = 1.f;
	side = -side;

	auto velocity = g_LocalPlayer->m_vecVelocity();

	QAngle wish_angle = cmd->viewangles;

	auto speed = velocity.Length2D();
	auto ideal_strafe = std::clamp(RAD2DEG(atan(15.f / speed)), 0.f, 90.f);

	cmd->forwardmove = 0.f;

	static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");

	static float old_yaw = 0.f;
	auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.f);
	auto abs_yaw_delta = abs(yaw_delta);
	old_yaw = wish_angle.yaw;

	const auto cl_sidespeed_value = cl_sidespeed->GetFloat();

	if (abs_yaw_delta <= ideal_strafe || abs_yaw_delta >= 30.f)
	{
		QAngle velocity_direction;
		Math::VectorAngles(velocity, velocity_direction);
		auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
		auto retrack = std::clamp(RAD2DEG(atan(30.f / speed)), 0.f, 90.f) * 2.f;
		if (velocity_delta <= retrack || speed <= 15.f)
		{
			if (-retrack <= velocity_delta || speed <= 15.0f)
			{
				wish_angle.yaw += side * ideal_strafe;
				cmd->sidemove = cl_sidespeed_value * side;
			}
			else
			{
				wish_angle.yaw = velocity_direction.yaw - retrack;
				cmd->sidemove = cl_sidespeed_value;
			}
		}
		else
		{
			wish_angle.yaw = velocity_direction.yaw + retrack;
			cmd->sidemove = -cl_sidespeed_value;
		}

		Math::CorrectMovement(cmd, wish_angle, cmd->viewangles);
	}
	else if (yaw_delta > 0.f)
		cmd->sidemove = -cl_sidespeed_value;
	else
		cmd->sidemove = cl_sidespeed_value;
}

void BunnyHop::Jumpbug(CUserCmd* cmd)
{
	float max_radias = 3.141592654 * 2;
	float step = max_radias / 128;
	float xThick = 23;
	if (g_Options.jump_bug && GetAsyncKeyState(g_Options.jump_bug_key)) {
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
			g_Options.misc_bhop2 = false;
			bool unduck = cmd->buttons &= ~in_duck;
			if (unduck) {
				cmd->buttons &= ~in_duck; // duck
				cmd->buttons |= in_jump; // jump
				unduck = false;
			}
			Vector pos = g_LocalPlayer->abs_origin();
			for (float a = 0.f; a < max_radias; a += step) {
				Vector pt;
				pt.x = (xThick * cos(a)) + pos.x;
				pt.y = (xThick * sin(a)) + pos.y;
				pt.z = pos.z;


				Vector pt2 = pt;
				pt2.z -= 8192;

				trace_t fag;

				Ray_t ray;
				ray.Init(pt, pt2);

				CTraceFilter flt;
				flt.pSkip = g_LocalPlayer;
				g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &fag);

				if (fag.fraction != 1.f && fag.fraction != 0.f) {
					cmd->buttons |= in_duck; // duck
					cmd->buttons &= ~in_jump; // jump
					unduck = true;
				}
			}
			for (float a = 0.f; a < max_radias; a += step) {
				Vector pt;
				pt.x = ((xThick - 2.f) * cos(a)) + pos.x;
				pt.y = ((xThick - 2.f) * sin(a)) + pos.y;
				pt.z = pos.z;

				Vector pt2 = pt;
				pt2.z -= 8192;

				trace_t fag;

				Ray_t ray;
				ray.Init(pt, pt2);

				CTraceFilter flt;
				flt.pSkip = g_LocalPlayer;
				g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &fag);

				if (fag.fraction != 1.f && fag.fraction != 0.f) {
					cmd->buttons |= in_duck; // duck
					cmd->buttons &= ~in_jump; // jump
					unduck = true;
				}
			}
			for (float a = 0.f; a < max_radias; a += step) {
				Vector pt;
				pt.x = ((xThick - 20.f) * cos(a)) + pos.x;
				pt.y = ((xThick - 20.f) * sin(a)) + pos.y;
				pt.z = pos.z;

				Vector pt2 = pt;
				pt2.z -= 8192;

				trace_t fag;

				Ray_t ray;
				ray.Init(pt, pt2);

				CTraceFilter flt;
				flt.pSkip = g_LocalPlayer;
				g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &fag);

				if (fag.fraction != 1.f && fag.fraction != 0.f) {
					cmd->buttons |= in_duck; // duck
					cmd->buttons &= ~in_jump; // jump
					unduck = true;
				}
			}
		}
	}
	else g_Options.misc_bhop2 = true;
}
