#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"

//increase it if valve added some funcs to baseentity :lillulmoa:
constexpr auto VALVE_ADDED_FUNCS = 0ull;

CGameRules* g_GameRules = nullptr;

bool C_BaseEntity::IsPlayer()
{
	//index: 152
	//ref: "effects/nightvision"
	//sig: 8B 92 ? ? ? ? FF D2 84 C0 0F 45 F7 85 F6
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 157 + VALVE_ADDED_FUNCS)(this);
}

bool C_BaseEntity::IsLoot() {
	return GetClientClass()->m_ClassID == ClassId_CPhysPropAmmoBox ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropLootCrate ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropRadarJammer ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		GetClientClass()->m_ClassID == ClassId_CDrone ||
		GetClientClass()->m_ClassID == ClassId_CDronegun ||
		GetClientClass()->m_ClassID == ClassId_CItem_Healthshot ||
		GetClientClass()->m_ClassID == ClassId_CItemCash || 
		GetClientClass()->m_ClassID == ClassId_CBumpMine;
}

bool C_BaseEntity::IsWeapon()
{
	//index: 160
	//ref: "CNewParticleEffect::DrawModel"
	//sig: 8B 80 ? ? ? ? FF D0 84 C0 74 6F 8B 4D A4
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 165 + VALVE_ADDED_FUNCS)(this);
}


bool C_BaseEntity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == ClassId_CPlantedC4;
}

bool C_BaseEntity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == ClassId_CBaseAnimating;
}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
	//return g_WeaponSystem->GetWpnData(this->m_Item().m_iItemDefinitionIndex());
	return CallVFunction<CCSWeaponInfo* (__thiscall*)(void*)>(this, 460)(this);

}

bool C_BaseCombatWeapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}

bool C_BaseCombatWeapon::CanFire()
{
	auto owner = this->m_hOwnerEntity().Get();
	if (!owner)
		return false;

	if (IsReloading() || m_iClip1() <= 0)
		return false;

	auto flServerTime = owner->m_nTickBase() * g_GlobalVars->interval_per_tick;

	//if ((ItemDefinitionIndex() == WEAPON_FAMAS || ItemDefinitionIndex() == WEAPON_GLOCK) && IsBurstMode() && GetBurstShotsRemaining() > 0)
	//	return true;

	if (owner->m_flNextAttack() > flServerTime)
		return false;

	//if (ItemDefinitionIndex() == WEAPON_REVOLVER && GetFireReadyTime() > flServerTime)
		//return false;


	return m_flNextPrimaryAttack() <= flServerTime;
}

bool C_BaseCombatWeapon::IsGrenade()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_GRENADE;
}

bool C_BaseCombatWeapon::IsGun()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_FISTS:
		return false;
	case WEAPONTYPE_HEALTHSHOT:
		return false;
	case WEAPONTYPE_TABLET:
		return false;
	case WEAPONTYPE_BREACHCHARGE:
		return false;
	case WEAPONTYPE_PLACEHOLDER:
		return false;
	default:
		return true;
	}
}

bool C_BaseCombatWeapon::IsKnife()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER) return false;
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsRifle()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_RIFLE:
		return true;
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	case WEAPONTYPE_SHOTGUN:
		return true;
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsPistol()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SNIPER:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::CanDoubletap()
{
	if (!this->IsGun())
		return false;

	auto idx = m_Item().m_iItemDefinitionIndex();
	if (idx == WEAPON_TASER || idx == WEAPON_REVOLVER || idx == WEAPON_SSG08 || idx == WEAPON_AWP || idx == WEAPON_XM1014 || idx == WEAPON_NOVA || idx == WEAPON_SAWEDOFF || idx == WEAPON_MAG7)
		return false;

	return true;
}

int C_BaseCombatWeapon::GetMaxTickbaseShift()
{
	if (!CanDoubletap())
		return g_GameRules->m_bIsValveDS() ? 6 : 16;

	auto idx = m_Item().m_iItemDefinitionIndex();
	auto max_tickbase_shift = 0;

	switch (idx)
	{
	case WEAPON_M249:
	case WEAPON_MAC10:
	case WEAPON_P90:
	case WEAPON_MP5:
	case WEAPON_NEGEV:
	case WEAPON_MP9:
		max_tickbase_shift = 5;
		break;
	case WEAPON_ELITE:
	case WEAPON_UMP45:
	case WEAPON_BIZON:
	case WEAPON_TEC9:
	case WEAPON_MP7:
		max_tickbase_shift = 6;
		break;
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_CZ75A:
		max_tickbase_shift = 7;
		break;
	case WEAPON_FIVESEVEN:
	case WEAPON_GLOCK:
	case WEAPON_P250:
	case WEAPON_SG556:
		max_tickbase_shift = 8;
		break;
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		max_tickbase_shift = 9;
		break;
	case WEAPON_DEAGLE:
		max_tickbase_shift = 13;
		break;
	case WEAPON_G3SG1:
	case WEAPON_SCAR20:
		max_tickbase_shift = 14;
		break;
	}

	if (g_GameRules->m_bIsValveDS())
		max_tickbase_shift = std::min(max_tickbase_shift, 6);

	return max_tickbase_shift;
}

bool C_BaseCombatWeapon::IsReloading()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
	return *(bool*)((uintptr_t)this + inReload);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 482 + VALVE_ADDED_FUNCS)(this);
}

float C_BaseCombatWeapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 452 + VALVE_ADDED_FUNCS)(this);
}

void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 483 + VALVE_ADDED_FUNCS)(this);
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
	return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
	return *(CUserCmd**)((uintptr_t)this + currentCommand);
}

int C_BasePlayer::GetNumAnimOverlays()
{
	return *(int*)((DWORD)this + 0x298C);
}

AnimationLayer *C_BasePlayer::GetAnimOverlays()
{
	return *(AnimationLayer**)((DWORD)this + 0x2990);
}

AnimationLayer *C_BasePlayer::GetAnimOverlay(int i)
{
	if (i < 15)
		return &GetAnimOverlays()[i];
	return nullptr;
}

int C_BasePlayer::GetSequenceActivity(int sequence)
{
	return -1;
}

CCSGOPlayerAnimState *C_BasePlayer::GetPlayerAnimState()
{
	return *(CCSGOPlayerAnimState**)((DWORD)this + 0x3914);
}

void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState *state, QAngle angle)
{
	static auto UpdateAnimState = Utils::PatternScan(
		GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");

	if (!UpdateAnimState)
		return;

	__asm {
		push 0
	}

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void C_BasePlayer::ResetAnimationState(CCSGOPlayerAnimState *state)
{
	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleA("client.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState *state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

Vector C_BasePlayer::GetEyePos()
{
	return m_vecOrigin() + m_vecViewOffset();
}

player_info_t C_BasePlayer::GetPlayerInfo()
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(EntIndex(), &info);
	return info;
}

bool C_BasePlayer::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE;
}

bool C_BasePlayer::IsFlashed()
{
	static auto m_flFlashMaxAlpha = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
	return *(float*)((uintptr_t)this + m_flFlashMaxAlpha - 0x8) > 200.0;
}
bool C_BasePlayer::IsEnemy()
{
	const bool is_dangerzone = Utils::IsDangerZone();

	if (is_dangerzone && g_LocalPlayer->m_nSurvivalTeam() == -1)
		return true;

	if (is_dangerzone)
		return g_LocalPlayer->m_nSurvivalTeam() != m_nSurvivalTeam();

	return g_LocalPlayer->m_iTeamNum() != m_iTeamNum();
}
bool C_BasePlayer::IsNotTarget() {
	if (!this || this == g_LocalPlayer)
		return true;

	if (!IsAlive() || IsDormant())
		return true;

	if (m_bGunGameImmunity())
		return true;

	if (m_fFlags() & FL_FROZEN)
		return true;

	if (!IsEnemy())
		return true;

	int entIndex = EntIndex();
	return entIndex > g_GlobalVars->maxClients;
}
bool C_BasePlayer::HasC4()
{
	static auto fnHasC4
		= reinterpret_cast<bool(__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client.dll"), "56 8B F1 85 F6 74 31")
			);

	return fnHasC4(this);
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}

mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				return hitbox;
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector &output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t *model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t *studioHdr = g_MdlInfo->GetStudiomodel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t *studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}

std::optional<Vector> C_BasePlayer::GetHitGroupPosition(int iHitGroup)
{
	assert(iHitGroup >= HITGROUP_GENERIC && iHitGroup <= HITGROUP_GEAR); // given invalid hitbox index for gethitgroupposition

	std::array<matrix3x4_t, MAXSTUDIOBONES> arrBonesToWorld = { };

	if (auto pModel = this->GetModel(); pModel != nullptr)
	{
		if (auto pStudioModel = g_MdlInfo->GetStudiomodel(pModel); pStudioModel != nullptr)
		{
			if (auto pHitboxSet = pStudioModel->GetHitboxSet(this->m_nHitboxSet()); pHitboxSet != nullptr)
			{
				if (this->SetupBones(arrBonesToWorld.data(), MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.f))
				{
					mstudiobbox_t* pHitbox = nullptr;
					for (int i = 0; i < pHitboxSet->numhitboxes; i++)
					{
						pHitbox = pHitboxSet->GetHitbox(i);

						// check is reached needed group
						if (pHitbox->group == iHitGroup)
							break;
					}

					if (pHitbox != nullptr)
					{
						Vector vecMin = { }, vecMax = { };

						// get mins/maxs by bone
						vecMin = Math::VectorTransform2(pHitbox->bbmin, arrBonesToWorld.at(pHitbox->bone));
						vecMax = Math::VectorTransform2(pHitbox->bbmax, arrBonesToWorld.at(pHitbox->bone));

						// get center
						return (vecMin + vecMax) * 0.5f;
					}
				}
			}
		}
	}

	return std::nullopt;
}


Vector C_BasePlayer::GetBonePos(int bone)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		return boneMatrix[bone].at(3);
	}
	return Vector{};
}
#include "../features/aimbot/backtrack.h"
Vector C_BasePlayer::GetBtBonePos(int bone)
{
	if (CLagCompensation::Get().data.count(this->EntIndex())) 	return Vector{};
	const auto records = CLagCompensation::Get().data.at(this->EntIndex());
	if (records.empty() || !CLagCompensation::Get().ValidTick(records.front().simTime)) 	return Vector{};

	if (SetupBones((matrix3x4_t*)records.back().boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		return records.back().boneMatrix[bone].at(3);
	}
	return Vector{};
}


bool  C_BasePlayer::IsVisible(C_BasePlayer* player, Vector start, Vector end)
{
	CGameTrace trace; Ray_t ray; CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(start, end);

	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

	return (trace.hit_entity == player);
}


bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	auto endpos = player->GetHitboxPos(hitbox);

	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

void C_BasePlayer::UpdateClientSideAnimation()
{
	return CallVFunction<void(__thiscall*)(void*)>(this, 223 + VALVE_ADDED_FUNCS)(this);
}
Vector C_BasePlayer::abs_origin()
{
	return CallVFunction<Vector& (__thiscall*)(void*)>(this, 10)(this);
}
int C_BasePlayer::GetHitboxNum()
{
	const auto pModel = GetModel();
	if (!pModel)
		return 0;

	studiohdr_t* sm = (studiohdr_t*)g_MdlInfo->GetStudiomodel(pModel);
	if (!sm)
		return 0;

	mstudiohitboxset_t* set = sm->GetHitboxSet(this->m_nHitboxSet());
	if (!set)
		return 0;

	return set->numhitboxes;

}
void C_BasePlayer::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)Utils::PatternScan(GetModuleHandleA("client.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	*(int*)((uintptr_t)this + 0xA30) = g_GlobalVars->framecount; //we'll skip occlusion checks now
	*(int*)((uintptr_t)this + 0xA28) = 0;//clear occlusion flags

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

int C_BasePlayer::m_nMoveType()
{
	return *(int*)((uintptr_t)this + 0x25C);
}

QAngle* C_BasePlayer::GetVAngles()
{
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	return (QAngle*)((uintptr_t)this + deadflag + 0x4);
}
void C_BaseAttributableItem::SetModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}
void C_BaseEntity::setModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

bool C_BasePlayer::CanSee(C_BasePlayer* other, const Vector& pos)
{
	const auto eyePos = GetEyePos();
	trace_t trace;
	Ray_t ray; ray.Init(eyePos, pos);
	CTraceFilter filter;
	filter.pSkip = this;
	g_EngineTrace->TraceRay(ray, 0x46004009, &filter, &trace);
	return trace.hit_entity == other || trace.fraction > 0.97f;
}


bool C_BasePlayer::VisibleTo(C_BasePlayer* Entity)
{
	if (!this || !Entity) return false;
	if (!this->IsAlive() || !Entity->IsAlive() || this->IsDormant() || Entity->IsDormant()) return false;

	if (Entity->CanSee(this, abs_origin() + Vector{ 0.0f, 0.0f, 5.0f }))
		return true;

	if (Entity->CanSee(this, GetEyePos() + Vector{ 0.0f, 0.0f, 5.0f }))
		return true;

	const auto model = GetModel();
	if (!model)
		return false;

	const auto studioModel = g_MdlInfo->GetStudiomodel(model);
	if (!studioModel)
		return false;

	const auto set = studioModel->GetHitboxSet(m_nHitboxSet());
	if (!set)
		return false;

	matrix3x4_t boneMatrices[MAXSTUDIOBONES];
	if (!SetupBones(boneMatrices, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
		return false;

	for (const auto boxNum : { 3, 15, 16 }) {
		const auto hitbox = set->GetHitbox(boxNum);
		if (hitbox && Entity->CanSee(this, boneMatrices[hitbox->bone].GetOrigin()))
			return true;
	}

	return false;
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 246 + VALVE_ADDED_FUNCS)(this, sequence);
}

float_t C_BasePlayer::m_flSpawnTime()
{
	return *(float_t*)((uintptr_t)this + 0xA370);
}
