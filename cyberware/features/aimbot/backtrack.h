#pragma once

#include "../../valve_sdk/sdk.hpp"
#include "../..//valve_sdk/csgostructs.hpp"
#include <map>
#include <deque>

#pragma region lagcompensation_definitions
#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
#define LAG_COMPENSATION_EPS_SQR ( 0.1f * 0.1f )
#define LAG_COMPENSATION_ERROR_EPS_SQR ( 4.0f * 4.0f )
#pragma endregion

#pragma region lagcompensation_enumerations
enum ELagCompensationState
{
	LC_NO = 0,
	LC_ALIVE = (1 << 0),
	LC_ORIGIN_CHANGED = (1 << 8),
	LC_ANGLES_CHANGED = (1 << 9),
	LC_SIZE_CHANGED = (1 << 10),
	LC_ANIMATION_CHANGED = (1 << 11)
};
#pragma endregion

struct SequenceObject_t
{
	SequenceObject_t(int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime)
		: iInReliableState(iInReliableState), iOutReliableState(iOutReliableState), iSequenceNr(iSequenceNr), flCurrentTime(flCurrentTime) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

struct StoredRecords {
	Vector origin;
	float simulation_time;
	matrix3x4_t matrix[128];
};


struct ConVarss {
	ConVar* update_rate;
	ConVar* max_update_rate;
	ConVar* interp;
	ConVar* interp_ratio;
	ConVar* min_interp_ratio;
	ConVar* max_interp_ratio;
	ConVar* max_unlag;
};

extern std::deque<StoredRecords> records[65];
extern ConVarss Cvars;

struct backtrack_data {
	float simTime;
	Vector hitboxPos, Origin;
	matrix3x4_t boneMatrix[128];
};


// @note: FYI - https://www.unknowncheats.me/forum/counterstrike-global-offensive/280912-road-perfect-aimbot-1-interpolation.html

class CLagCompensation : public Singleton<CLagCompensation>
{
public:
	float correct_time = 0.0f;
	float latency = 0.0f;
	float lerp_time = 0.0f;
	std::map<int, std::deque<backtrack_data>> data;

	// Get
	void Run(CUserCmd* pCmd);
	void Update();
	float GetLerpTime();
	int TimeToTicks(float time);
	bool ValidTick(float simtime);
	float GetDeltaTime(float simtime);
	const std::deque<backtrack_data>* GetRecords(std::size_t index);

	// Main
	void UpdateIncomingSequences(INetChannel* pNetChannel);
	void ClearIncomingSequences();
	void AddLatencyToNetChannel(INetChannel* pNetChannel);

	static void Init() {
		records->clear();

		Cvars.update_rate = g_CVar->FindVar("cl_updaterate");
		Cvars.max_update_rate = g_CVar->FindVar("sv_maxupdaterate");
		Cvars.interp = g_CVar->FindVar("cl_interp");
		Cvars.interp_ratio = g_CVar->FindVar("cl_interp_ratio");
		Cvars.min_interp_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
		Cvars.max_interp_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");
		Cvars.max_unlag = g_CVar->FindVar("sv_maxunlag");
	}
private:
	// Values
	std::deque<SequenceObject_t> vecSequences = { };
	/* our real incoming sequences count */
	int nRealIncomingSequence = 0;
	/* count of incoming sequences what we can spike */
	int nLastIncomingSequence = 0;
};
