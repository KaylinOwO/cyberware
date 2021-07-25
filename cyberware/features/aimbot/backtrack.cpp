#include "backtrack.h"
#include <algorithm>
#include "../..//helpers/math.hpp"
#include "../../options.hpp"

std::deque<StoredRecords> records[65];
ConVarss Cvars;

#undef max
float CLagCompensation::GetLerpTime() {
	auto ratio = std::clamp(Cvars.interp_ratio->GetFloat(), Cvars.min_interp_ratio->GetFloat(), Cvars.max_interp_ratio->GetFloat());
	return (std::max)(Cvars.interp->GetFloat(), (ratio / ((Cvars.max_update_rate) ? Cvars.max_update_rate->GetFloat() : Cvars.update_rate->GetFloat())));
}

int CLagCompensation::TimeToTicks(float time) {
	return static_cast<int>((0.5f + time / g_GlobalVars->interval_per_tick));
}

float CLagCompensation::GetDeltaTime(float simtime)
{
	auto network = g_EngineClient->GetNetChannelInfo();

	auto delta = std::clamp(network->GetLatency(0) + GetLerpTime(), 0.f, Cvars.max_unlag->GetFloat()) - (g_GlobalVars->curtime - simtime);

	return delta;
}


float GetServerTime(CUserCmd* cmd)
{
	if (!cmd) return 0.0f;
	static int tick;
	static CUserCmd* lastCmd;

	if (cmd) {
		if (g_LocalPlayer && (!lastCmd || lastCmd->hasbeenpredicted))
			tick = g_LocalPlayer->m_nTickBase();
		else
			tick++;
		lastCmd = cmd;
	}
	return tick * g_GlobalVars->interval_per_tick;
}


bool CLagCompensation::ValidTick(float simtime) {
	auto network = g_EngineClient->GetNetChannelInfo();
	if (!network)
		return false;

	auto delta = std::clamp(network->GetLatency(0) + GetLerpTime(), 0.f, Cvars.max_unlag->GetFloat()) - (g_GlobalVars->curtime - simtime);
	//auto delta = std::clamp(network->GetLatency(0) + network->GetLatency(1) + GetLerpTime(), 0.f, Cvars.max_unlag->GetFloat()) - (GetServerTime(g_pCmd) - simtime);
	return std::fabsf(delta) <= 0.2f;
}

const std::deque<backtrack_data>* CLagCompensation::GetRecords(std::size_t index)
{
	if (!g_Options.misc_bt)
		return nullptr;
	return &data.at(index);
}

float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle) {
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}


void CLagCompensation::Run(CUserCmd* pCmd)
{
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->IsDormant()) {
		data.clear();
		return;
	}

	if (!g_Options.misc_bt) {
		data.clear();
		return;
	}

	static ConVar* sv_maxunlag = g_CVar->FindVar("sv_maxunlag");
	static ConVar* sv_minupdaterate = g_CVar->FindVar("sv_minupdaterate");
	static ConVar* sv_maxupdaterate = g_CVar->FindVar("sv_maxupdaterate");

	static ConVar* sv_client_min_interp_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	static ConVar* sv_client_max_interp_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

	static ConVar* cl_interp_ratio = g_CVar->FindVar("cl_interp_ratio");
	static ConVar* cl_interp = g_CVar->FindVar("cl_interp");
	static ConVar* cl_updaterate = g_CVar->FindVar("cl_updaterate");

	float updaterate = cl_updaterate->GetFloat();

	float minupdaterate = sv_minupdaterate->GetFloat();
	float maxupdaterate = sv_maxupdaterate->GetFloat();

	float min_interp = sv_client_min_interp_ratio->GetFloat();
	float max_interp = sv_client_max_interp_ratio->GetFloat();

	float flLerpAmount = cl_interp->GetFloat();
	float flLerpRatio = cl_interp_ratio->GetFloat();
	flLerpRatio = std::clamp(flLerpRatio, min_interp, max_interp);
	if (flLerpRatio == 0.0f) flLerpRatio = 1.0f;

	float updateRate = std::clamp(updaterate, minupdaterate, maxupdaterate);
	lerp_time = std::fmaxf(flLerpAmount, flLerpRatio / updateRate);
	latency = g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);
	correct_time = latency + lerp_time;

	C_BasePlayer* player;
	for (int i = 1; i <= g_GlobalVars->maxClients; ++i) {
		player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player) {
			if (data.count(i) > 0) data.erase(i);
			continue;
		}

		if (player->IsDormant() || !player->IsAlive()) {
			if (data.count(i) > 0) data.erase(i);
			continue;
		}

		if (!player->IsPlayer() || !player->IsEnemy()) {
			if (data.count(i) > 0) data.erase(i);
			continue;
		}

		auto& cur_data = data[i];
		if (!cur_data.empty()) {
			auto& front = cur_data.front();
			if (front.simTime == player->m_flSimulationTime()) continue;

			while (!cur_data.empty()) {
				auto& back = cur_data.back();
				float deltaTime = correct_time - (g_GlobalVars->curtime - back.simTime);
				if (std::fabsf(deltaTime) <= 0.2f) break;

				cur_data.pop_back();
			}
		}

		auto model = player->GetModel();
		if (!model) continue;

		auto hdr = g_MdlInfo->GetStudiomodel(model);
		if (!hdr) continue;

		auto hitbox_set = hdr->GetHitboxSet(player->m_nHitboxSet());
		auto hitbox_head = hitbox_set->GetHitbox(HITBOX_HEAD);
		auto hitbox_center = (hitbox_head->bbmin + hitbox_head->bbmax) * 0.5f;

		backtrack_data bd;
		bd.simTime = player->m_flSimulationTime();

		*(Vector*)((uintptr_t)player + 0xA0) = player->m_vecOrigin();
		*(int*)((uintptr_t)player + 0xA68) = 0;
		*(int*)((uintptr_t)player + 0xA30) = 0;
		player->InvalidateBoneCache();
		player->SetupBones(bd.boneMatrix, 128, 256, g_GlobalVars->curtime);

		Math::VectorTransform(hitbox_center, bd.boneMatrix[hitbox_head->bone], bd.hitboxPos);
		bd.Origin = player->abs_origin();

		data[i].push_front(bd);
	}

	Vector localEyePos = g_LocalPlayer->GetEyePos();
	QAngle angles;
	int tick_count = -1;
	float best_fov = 180.0f;

	for (auto& node : data) {
		auto& cur_data = node.second;
		if (cur_data.empty()) continue;

		for (auto& bd : cur_data) {
			float deltaTime = correct_time - (g_GlobalVars->curtime - bd.simTime);
			if (std::fabsf(deltaTime) > (g_Options.extended_bt ? 400.0f : g_Options.bt_ticks) ) continue;

			Math::VectorAngles(bd.hitboxPos - localEyePos, angles);
			Math::FixAngles(angles);
			float fov = GetFovToPlayer(pCmd->viewangles, angles);
			if (best_fov > fov) {
				best_fov = fov;
				tick_count = TIME_TO_TICKS(bd.simTime + lerp_time);
			}
		}
	}

	if (tick_count != -1) pCmd->tick_count = tick_count;
}

void CLagCompensation::Update()
{
	if (!g_Options.misc_bt || !g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->IsDormant()) {
		if (!records->empty())
			records->clear();
		return;
	}

	for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++) {
		C_BasePlayer* entity = C_BasePlayer::GetPlayerByIndex(i);
		if (!entity || entity == g_LocalPlayer || entity->IsDormant() || !entity->IsAlive() || !entity->IsEnemy()) {
			records[i].clear();
			continue;
		}

		if (!records[i].empty() && (records[i].front().simulation_time == entity->m_flSimulationTime()))
			continue;

		StoredRecords record{ };
		record.origin = entity->abs_origin();
		record.simulation_time = entity->m_flSimulationTime();

		entity->SetupBones(record.matrix, 128, 0x7FF00, g_GlobalVars->curtime);

		records[i].push_front(record);

		float Ticks = (g_Options.extended_bt ? 380.0f : g_Options.bt_ticks);
		while (records[i].size() > 3 && records[i].size() > static_cast<size_t>(TimeToTicks(static_cast<float>(Ticks) / 1000.f)))
			records[i].pop_back();

		if (auto invalid = std::find_if(std::cbegin(records[i]), std::cend(records[i]), [](const StoredRecords& rec) { return !CLagCompensation::Get().ValidTick(rec.simulation_time); }); invalid != std::cend(records[i]))
			records[i].erase(invalid, std::cend(records[i]));
	}
}

void CLagCompensation::UpdateIncomingSequences(INetChannel* pNetChannel)
{
	if (pNetChannel == nullptr)
		return;

	// set to real sequence to update, otherwise needs time to get it work again
	if (nLastIncomingSequence == 0)
		nLastIncomingSequence = pNetChannel->m_nInSequenceNr;

	// check how much sequences we can spike
	if (pNetChannel->m_nInSequenceNr > nLastIncomingSequence)
	{
		nLastIncomingSequence = pNetChannel->m_nInSequenceNr;
		vecSequences.emplace_front(SequenceObject_t(pNetChannel->m_nInReliableState, pNetChannel->m_nOutReliableState, pNetChannel->m_nInSequenceNr, g_GlobalVars->realtime));
	}

	// is cached too much sequences
	if (vecSequences.size() > 2048U)
		vecSequences.pop_back();
}

void CLagCompensation::ClearIncomingSequences()
{
	if (!vecSequences.empty())
	{
		nLastIncomingSequence = 0;
		vecSequences.clear();
	}
}

void CLagCompensation::AddLatencyToNetChannel(INetChannel* pNetChannel)
{
	INetChannelInfo* pNetChannelInfo = g_EngineClient->GetNetChannelInfo();
	static ConVar* sv_maxunlag = g_CVar->FindVar("sv_maxunlag");
	if (pNetChannelInfo == nullptr || sv_maxunlag == nullptr)
		return;

	const float flMaxLatency = std::max(0.f, std::clamp(0.4f, 0.f, sv_maxunlag->GetFloat()) - pNetChannelInfo->GetLatency(FLOW_OUTGOING));
	for (const auto& sequence : vecSequences)
	{
		if (g_GlobalVars->realtime - sequence.flCurrentTime >= flMaxLatency)
		{
			pNetChannel->m_nInReliableState = sequence.iInReliableState;
			pNetChannel->m_nInSequenceNr = sequence.iSequenceNr;
			break;
		}
	}
}
