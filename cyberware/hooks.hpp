#pragma once
#include "valve_sdk/sdk.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/vfunc_hook.hpp"
#include <d3d9.h>
#include "features/aimbot/aimbot.hpp"
#include "helpers/prediction.hpp"

namespace index
{
	constexpr auto EmitSound1               = 5;
	constexpr auto EmitSound2               = 6;
    constexpr auto EndScene                 = 42;
    constexpr auto Reset                    = 16;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 24;
	constexpr auto DrawModelExecute2        = 29;
    constexpr auto PlaySound                = 82;
    constexpr auto FrameStageNotify         = 37;
    constexpr auto DrawModelExecute         = 21;
    constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto OverrideView             = 18;
	constexpr auto LockCursor               = 67;
	constexpr auto RenderSmokeOverlay       = 41;
	constexpr auto FireEvent                = 9;

}

namespace Hooks
{
    void Initialize();
    void Shutdown();

    inline vfunc_hook hlclient_hook;
	inline vfunc_hook direct3d_hook;
	inline vfunc_hook vguipanel_hook;
	inline vfunc_hook vguisurf_hook;
	inline vfunc_hook mdlrender_hook;
	inline vfunc_hook viewrender_hook;
	inline vfunc_hook enginemode_hook;
	inline vfunc_hook rpe_hook;
	inline vfunc_hook sound_hook;
	inline vfunc_hook clientmode_hook;
	inline vfunc_hook sv_cheats;
	inline vfunc_hook stdrender_hook;
	inline vfunc_hook hk_netchannel;
	inline vfunc_hook gameevents_hook;
	inline recv_prop_hook* sequence_hook;
	typedef bool(__thiscall* sendnetmsg_fn)(void*, INetMessage* msg, bool reliable, bool voice);
	inline sendnetmsg_fn original_sendnetmsg = nullptr;

	typedef int(__thiscall* senddatagram_fn)(INetChannel* thisptr, bf_write* pDatagram);
	inline senddatagram_fn original_senddatagram = nullptr;

	typedef void(__thiscall* rpe_fn)(void* client_state, void* ent_info);
	inline rpe_fn original_rpe = nullptr;

	using FireEvent = bool(__thiscall*)(IGameEventManager2*, IGameEvent* pEvent);
    long __stdcall hkEndScene(IDirect3DDevice9* device);
    long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
    bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd);
	bool __fastcall hkIsHLTV(uintptr_t ecx, uintptr_t edx);
	void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	void __fastcall hkEmitSound1(void* _this, int, IRecipientFilter & filter, int iEntIndex, int iChannel, const char * pSoundEntry, unsigned int nSoundEntryHash, const char * pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector * pOrigin, const Vector * pDirection, void * pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
    void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
    void __fastcall hkFrameStageNotify(void* _this, int, ClientFrameStage_t stage);
	void __fastcall hkOverrideView(void* _this, int, CViewSetup * vsView);
	void __fastcall hkLockCursor(void* _this);
    int  __fastcall hkDoPostScreenEffects(void* _this, int, int a1);
	bool __fastcall hkSvCheatsGetBool(void* pConVar, void* edx);
	void __fastcall hkDrawModelExecute2(void* _this, int, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags);
	void __fastcall RenderSmokeOverlay(void* _this, int, const bool unk);
	bool _fastcall send_net_msg(void* ecx, void* edx, INetMessage* msg, bool reliable, bool voice);
	bool __stdcall hkFireEvent(IGameEvent* pEvent);
	void __fastcall RPEHk(void* client_state, void* edx, void* ent_info);

	void RecvProxy(const CRecvProxyData* pData, void* entity, void* output);
}
