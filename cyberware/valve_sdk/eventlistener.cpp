#include "eventlistener.h"
#include "csgostructs.hpp"
#include "Misc/fnv1a.h"
#include "../options.hpp"
#include "../features/visuals/visuals.hpp"

void CEventListener::Setup(const std::deque<const char*>& arrEvents)
{
	if (arrEvents.empty())
		return;

	for (auto szEvent : arrEvents)
	{
		g_GameEvents->AddListener(this, szEvent, false);
		
		if (!g_GameEvents->FindListener(this, szEvent))
			throw std::runtime_error("failed add listener");
	}
}


void CEventListener::Destroy()
{
	g_GameEvents->RemoveListener(this);
}

void KillSound(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!g_EngineClient->IsInGame())
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (uNameHash != FNV1A::HashConst("player_death"))
		return;

	const float flServerTime = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase());

	/* get hitmarker info */
	C_BasePlayer* pAttacker = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")));
	if (pAttacker == g_LocalPlayer)
	{
		C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")));
		if (pEntity != nullptr && pEntity != g_LocalPlayer)
		{
			if (g_Options.killsound)
				g_VGuiSurface->PlaySound_("cwkillsound.wav");

			const auto vecPosition = pEntity->GetHitGroupPosition(pEvent->GetInt(XorStr("hitgroup").c_str()));

			if (!vecPosition.has_value())
				return;

			// add hit info
			Visuals::Get().vecHitMarks.emplace_back(HitMarkerObject_t{ vecPosition.value(), pEvent->GetInt(XorStr("dmg_health").c_str()), flServerTime + 1.0f });
		}

	}
}

void HitSound(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!g_EngineClient->IsInGame())
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (uNameHash != FNV1A::HashConst("player_hurt"))
		return;

	const float flServerTime = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase());

	/* get hitmarker info */
	C_BasePlayer* pAttacker = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")));
	if (pAttacker == g_LocalPlayer)
	{
		C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")));
		if (pEntity != nullptr && pEntity != g_LocalPlayer)
		{
			if (g_Options.hitsound)
				g_VGuiSurface->PlaySound_("cwhitsound.wav");

			const auto vecPosition = pEntity->GetHitGroupPosition(pEvent->GetInt(XorStr("hitgroup").c_str()));

			if (!vecPosition.has_value())
				return;

			// add hit info
			Visuals::Get().vecHitMarks.emplace_back(HitMarkerObject_t{ vecPosition.value(), pEvent->GetInt(XorStr("dmg_health").c_str()), flServerTime + 1.0f });
		}

	}
}


void CEventListener::FireGameEvent(IGameEvent* pEvent)
{
	if (pEvent == nullptr)
		return;

	// get hash of event name
	const FNV1A_t uNameHash = FNV1A::Hash(pEvent->GetName());
	HitSound(pEvent, uNameHash);
	KillSound(pEvent, uNameHash);

}