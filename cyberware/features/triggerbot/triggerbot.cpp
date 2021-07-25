#include "triggerbot.h"
#include "../../helpers/math.hpp"
#include "../aimbot/backtrack.h"

static std::optional<Vector> GetIntersectionPoint(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, float radius)
{
	auto sphereRayIntersection = [start, end, radius](auto&& center) -> std::optional<Vector>
	{
		auto direction = (end - start).Normalized();

		auto q = center - start;
		auto v = q.DotProduct(direction);
		auto d = radius * radius - (q.LengthSqr() - v * v);

		if (d < FLT_EPSILON)
			return {};

		return start + direction * (v - std::sqrt(d));
	};

	auto delta = (maxs - mins).Normalized();
	for (size_t i{}; i < std::floor(mins.DistTo(maxs)); ++i)
	{
		if (auto intersection = sphereRayIntersection(mins + delta * float(i)); intersection)
			return intersection;
	}

	if (auto intersection = sphereRayIntersection(maxs); intersection)
		return intersection;

	return {};
}

static std::optional<backtrack_data> GetTargetRecord(C_BasePlayer* entity, const Vector& start, const Vector& end)
{
	auto hdr = g_MdlInfo->GetStudiomodel(entity->GetModel());
	if (!hdr)
		return {};

	auto set = hdr->GetHitboxSet(entity->m_nHitboxSet());
	if (!set)
		return {};

	if (CLagCompensation::Get().data.count(entity->EntIndex()))
	{
		for (auto& record : CLagCompensation::Get().data.at(entity->EntIndex()))
		{
			for (size_t i{}; i < set->numhitboxes; ++i)
			{
				auto hitbox = set->GetHitbox(i);
				if (!hitbox || hitbox->m_flRadius == -1.f)
					continue;

				Vector mins, maxs;
				Math::VectorTransform(hitbox->bbmin, record.boneMatrix[hitbox->bone], mins);
				Math::VectorTransform(hitbox->bbmax, record.boneMatrix[hitbox->bone], maxs);

				if (auto intersection = GetIntersectionPoint(start, end, mins, maxs, hitbox->m_flRadius); intersection)
				{
					Ray_t ray;
					CGameTrace trace;
					CTraceFilter filter;

					filter.pSkip = g_LocalPlayer;
					ray.Init(start, *intersection);

					g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
					if (g_Options.trigger_head && trace.hitgroup == HITGROUP_HEAD ||
						// chest
						g_Options.trigger_chest && trace.hitgroup == HITGROUP_CHEST ||
						// stomach
						g_Options.trigger_stomach && trace.hitgroup == HITGROUP_STOMACH ||
						// arms
						g_Options.trigger_arms && (trace.hitgroup == HITGROUP_LEFTARM || trace.hitgroup == HITGROUP_RIGHTARM) ||
						// legs
						g_Options.trigger_legs && (trace.hitgroup == HITGROUP_LEFTLEG || trace.hitgroup == HITGROUP_RIGHTLEG))
					{
						if (!trace.DidHit()) //position isn't behind wall
							return record;
					}
				}
			}
		}
	}


	return {};
}


void CTriggerBot::Run(CUserCmd* cmd)
{
	if (!g_LocalPlayer->IsAlive() || g_LocalPlayer->IsDormant())
		return;

	static ConVar* weapon_recoil_scale = g_CVar->FindVar(XorStr("weapon_recoil_scale").c_str());

	if (weapon_recoil_scale == nullptr)
		return;

	auto pWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!pWeapon || !pWeapon->IsGun())
		return;

	auto pWeaponData = pWeapon->GetCSWeaponData();
	if (!pWeaponData)
		return;


	static int ticks;
	ticks++;

	if (g_Options.trigger_key > 0 && !GetAsyncKeyState(g_Options.trigger_key))
	{
		ticks = 0;
		return;
	}

	QAngle angView = cmd->viewangles;
	angView += g_LocalPlayer->m_aimPunchAngle() * weapon_recoil_scale->GetFloat();

	Vector vecStart, vecEnd, vecForward;
	Math::AngleVectors(angView, vecForward);

	vecStart = g_LocalPlayer->GetEyePos();
	vecForward *= pWeaponData->flRange;
	vecEnd = vecStart + vecForward;

/*	for (auto i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		auto entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!entity || !entity->IsEnemy() || !entity->IsAlive() || entity->IsDormant() || entity->m_bGunGameImmunity())
		{
			ticks = 0;
			continue;
		}

		auto record = GetTargetRecord(entity, vecStart, vecEnd);
		if (!record) { ticks = 0; continue; }


		if (pWeapon->CanFire())
			cmd->buttons |= IN_ATTACK;

	}*/

	CGameTrace trace;
	Ray_t ray; ray.Init(vecStart, vecEnd);
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;
	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

	C_BasePlayer* pEntity = (C_BasePlayer*)trace.hit_entity;

	// check is trace player valid and enemy
	if (!pEntity)
	{
		ticks = 0;
		return;
	}

	if (!pEntity->IsAlive() || pEntity->IsDormant() || !pEntity->IsPlayer() || pEntity->m_bGunGameImmunity() || !pEntity->IsEnemy())
	{
		ticks = 0;
		return;
	}

	ticks++;

	// hitgroup filters check
		// head

	if (g_Options.trigger_head && trace.hitgroup == HITGROUP_HEAD ||
		// chest
		g_Options.trigger_chest && trace.hitgroup == HITGROUP_CHEST ||
		// stomach
		g_Options.trigger_stomach && trace.hitgroup == HITGROUP_STOMACH ||
		// arms
		g_Options.trigger_arms && (trace.hitgroup == HITGROUP_LEFTARM || trace.hitgroup == HITGROUP_RIGHTARM) ||
		// legs
		g_Options.trigger_legs && (trace.hitgroup == HITGROUP_LEFTLEG || trace.hitgroup == HITGROUP_RIGHTLEG))
	{
		if (pWeapon->CanFire())
		{
			// check is delay elapsed
			if (g_Options.trigger_delay > 0)
			{
				if (ticks < g_Options.trigger_delay)
					return;
			}

			cmd->buttons |= IN_ATTACK;
		}
	}
}
