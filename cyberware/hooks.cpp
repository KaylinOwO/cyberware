#include "hooks.hpp"
#include <intrin.h>  
#include "main.h"
#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "helpers/security/security.h"

#include "features/aimbot/aimbot.hpp"
#include "features/triggerbot/triggerbot.h"
#include "features/aimbot/backtrack.h"
#include "features/exploits/exploits.h"

#include "features/visuals/visuals.hpp"
#include "features/visuals/chams.hpp"
#include "features/visuals/glow.hpp"
#include "features/visuals/grenade_prediction.h"
#include "features/skins.h"

#include "features/misc/misc.h"
#include "features/misc/bhop.hpp"

#include "features/discord-rpc/discord-rpc.h"	

#pragma intrinsic(_ReturnAddress)  

namespace Hooks {

	void Initialize()
	{
		auto rpe_targ = (void*)((uintptr_t)g_ClientState + 0x8);

		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		stdrender_hook.setup(g_StudioRender);
		viewrender_hook.setup(g_ViewRender);
		enginemode_hook.setup(g_EngineClient);
		rpe_hook.setup(rpe_targ);

		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		gameevents_hook.setup(g_GameEvents);
		gameevents_hook.hook_index(index::FireEvent, hkFireEvent);
		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		enginemode_hook.hook_index(93, hkIsHLTV);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		clientmode_hook.hook_index(index::CreateMove, hkCreateMove);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		stdrender_hook.hook_index(index::DrawModelExecute2, hkDrawModelExecute2);
		viewrender_hook.hook_index(index::RenderSmokeOverlay, RenderSmokeOverlay);
		rpe_hook.hook_index(64, RPEHk);

		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), RecvProxy);
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
		sequence_hook->~recv_prop_hook();

	}

	int __fastcall send_datagram(INetChannel* thisptr, int edx, bf_write* pDatagram)
	{
		static auto oSendDatagram = hk_netchannel.get_original<senddatagram_fn>(46);
		//static auto oSendDatagram = DTR::SendDatagram.GetOriginal<decltype(&hkSendDatagram)>();

		if (!g_EngineClient->IsInGame() || !g_Options.extended_bt || pDatagram != nullptr)
			return oSendDatagram(thisptr, pDatagram);

		const int iInReliableStateOld = thisptr->m_nInReliableState;
		const int iInSequenceNrOld = thisptr->m_nInSequenceNr;

		CLagCompensation::Get().AddLatencyToNetChannel(thisptr);

		const int iReturn = oSendDatagram(thisptr, pDatagram);

		thisptr->m_nInReliableState = iInReliableStateOld;
		thisptr->m_nInSequenceNr = iInSequenceNrOld;

		return iReturn;
	}

	bool __fastcall send_net_msg(void* ecx, void* edx, INetMessage* msg, bool reliable, bool voice)
	{
		static auto oFireEvent = hk_netchannel.get_original<sendnetmsg_fn>(40);

		if (!msg)
			return original_sendnetmsg(ecx, msg, reliable, voice);

		if (msg->GetType() == 14)
			return false;

		if (msg->GetGroup() == INetChannelInfo::VOICE)
			voice = true;

		return oFireEvent(ecx, msg, reliable, voice);
	}

	bool __stdcall hkFireEvent(IGameEvent* pEvent) {
		static auto oFireEvent = gameevents_hook.get_original<FireEvent>(index::FireEvent);

		if (!pEvent)
			return oFireEvent(g_GameEvents, pEvent);

		const char* szEventName = pEvent->GetName();


		if (!strcmp(szEventName, "server_spawn"))
		{

			const auto net_channel = g_EngineClient->GetNetChannelInfo();

			if (net_channel != nullptr)
			{
				const auto index = 40;
				Hooks::hk_netchannel.setup(net_channel);
				Hooks::hk_netchannel.hook_index(index, send_net_msg);
				Hooks::hk_netchannel.hook_index(46, send_datagram);
			}
		}

		if (!strcmp(szEventName, "cs_game_disconnected"))
		{
			if (hk_netchannel.is_hooked())
			{
				hk_netchannel.unhook_all();
			}
		}

		return oFireEvent(g_GameEvents, pEvent);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		auto esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();
	

		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}
	//--------------------------------------------------------------------------------

	float real_angle = 0.0f;
	float view_angle = 0.0f;
	static int max_choke_ticks = 14;
	static CCSGOPlayerAnimState g_AnimState;
	bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd)
	{
		bool* SendPacket = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(_AddressOfReturnAddress()) + 0x14);
		static auto oCreateMove = clientmode_hook.get_original<decltype(&hkCreateMove)>(index::CreateMove);
		oCreateMove(input_sample_frametime, cmd);

		if (!cmd || !cmd->command_number) return false;
		g_pCmd = cmd;

		QAngle oldAngle = cmd->viewangles;
		static int latency_ticks = 0;
		float fl_latency = g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		int latency = TIME_TO_TICKS(fl_latency);;
		if (g_ClientState->m_nChokedCommands <= 0) latency_ticks = latency;
		else latency_ticks = std::max(latency, latency_ticks);

		if (g_GameRules->m_bIsValveDS()) {
			if (fl_latency >= g_GlobalVars->interval_per_tick) max_choke_ticks = 6 - latency_ticks;
			else max_choke_ticks = 6;
		}
		else max_choke_ticks = 13 - latency_ticks;

		static float SpawnTime = 0.0f;
		if (g_LocalPlayer->m_flSpawnTime() != SpawnTime) {
			g_AnimState.pBaseEntity = g_LocalPlayer;
			g_LocalPlayer->ResetAnimationState(&g_AnimState);
			SpawnTime = g_LocalPlayer->m_flSpawnTime();
		}
		
		if (Menu::Get().IsVisible()) cmd->buttons &= ~IN_ATTACK | IN_ATTACK2;

		INetChannel* pNetChannel = (INetChannel*)g_ClientState->m_NetChannel;

		BunnyHop::OnCreateMove(cmd);
		BunnyHop::AutoStafe(cmd);

		Utils::Clantag();

		static auto prediction = new PredictionSystem();
		auto flags = g_LocalPlayer->m_fFlags();

		/*uintptr_t* frame_ptr;
		__asm mov frame_ptr, ebp;
		static auto should_recharge = false;
		if (should_recharge)
		{
			++g_TicksAllowed;

			cmd->tick_count = INT_MAX;
			cmd->forwardmove = 0.0f;
			cmd->sidemove = 0.0f;
			cmd->upmove = 0.0f;
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons &= ~IN_ATTACK2;

			if (g_TicksAllowed >= 16)
			{
				should_recharge = false;
				*(bool*)(*frame_ptr - 0x1C) = true;
			}
			else
				*(bool*)(*frame_ptr - 0x1C) = false;

			return false;
		}

		if (g_TicksAllowed < 16 && (g_Options.Doubletap && g_Options.DoubletapKey || g_Options.HideShots && g_Options.HideShotsKey))
			should_recharge = true;

		g_BackupTickbase = g_LocalPlayer->m_nTickBase();

		if (g_NextTickbaseShift)
			g_FixedTickbase = g_LocalPlayer->m_nTickBase() - g_NextTickbaseShift;
		else
			g_FixedTickbase = g_BackupTickbase;

		g_TickbaseShift = 0;
		g_DoubletapFire = false;
		*/
		prediction->StartPrediction(cmd);
		{
			g_Legitbot->Run(cmd);
			g_Triggerbot->Run(cmd);
			CLagCompensation::Get().Run(cmd);
			GrenadePrediction::trace(cmd);
			BunnyHop::Jumpbug(cmd);
			if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND) && g_LocalPlayer->IsAlive()) if (g_Options.edge_bug && GetAsyncKeyState(g_Options.edge_bug_key)) cmd->buttons = 4;
			/*	static bool edgebugged = false;
	if (g_LocalPlayer->m_vecVelocity().z >= -7 || g_LocalPlayer->m_fFlags() & FL_ONGROUND)
	{
		edgebugged = false;
	}
	else
	{
		if (g_Options.edge_bug && GetAsyncKeyState(g_Options.edge_bug_key))
		{
			edgebugged = true;
			g_EngineClient->ClientCmd_Unrestricted("say I FUCKING EDGEBUGGED");
		}

	}*/

			if (std::fabsf(g_LocalPlayer->m_flSpawnTime() - g_GlobalVars->curtime) > 1.0f)
				CMisc::Desync(cmd, SendPacket);

			Math::FixAngles(cmd->viewangles);
			cmd->viewangles.yaw = std::remainderf(cmd->viewangles.yaw, 360.0f);

			if (g_Options.Desync && g_ClientState->m_nChokedCommands >= max_choke_ticks) {
				*SendPacket = true;
				cmd->viewangles = g_ClientState->viewangles;
			}

			CMisc::CorrectMouse(cmd);

			auto anim_state = g_LocalPlayer->GetPlayerAnimState();
			if (anim_state) {
				CCSGOPlayerAnimState anim_state_backup = *anim_state;
				*anim_state = g_AnimState;
				*g_LocalPlayer->GetVAngles() = cmd->viewangles;
				g_LocalPlayer->UpdateClientSideAnimation();

				if (anim_state->speed_2d > 0.1f || std::fabsf(anim_state->flUpVelocity)) {
					CMisc::next_lby = g_GlobalVars->curtime + 0.22f;
				}
				else if (g_GlobalVars->curtime > CMisc::next_lby) {
					if (std::fabsf(Utils::AngleDiff(anim_state->m_flGoalFeetYaw, anim_state->m_flEyeYaw)) > 35.0f)
						CMisc::next_lby = g_GlobalVars->curtime + 1.1f;
				}

				g_AnimState = *anim_state;
				*anim_state = anim_state_backup;
			}

			if (*SendPacket) {
				real_angle = g_AnimState.m_flGoalFeetYaw;
				view_angle = g_AnimState.m_flEyeYaw;
			}

		/*	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
			if (pWeapon && pWeapon->IsGun()){
				auto double_tap_aim_check = false;

				if (cmd->buttons & IN_ATTACK && g_DoubletapAimCheck)
				{
					double_tap_aim_check = true;
					g_DoubletapAimCheck = false;
				}

				//auto revolver_shoot = pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2);

				if (cmd->buttons & IN_ATTACK && pWeapon->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER)
				{
					if (!double_tap_aim_check)
						g_DoubletapAim = false;
				}
			}

			auto backup_ticks_allowed = g_TicksAllowed;
			//if (misc::get().double_tap(m_pcmd))
			//	misc::get().hide_shots(m_pcmd, false);
			//else
			//{
			//	g_TicksAllowed = backup_ticks_allowed;
			//	misc::get().hide_shots(m_pcmd, true);
			//}

			CExploits::Doubletap(cmd);

			if (pWeapon && pWeapon->IsGun()) {
				auto double_tap_aim_check = false;

				if (cmd->buttons & IN_ATTACK && g_DoubletapAimCheck)
				{
					double_tap_aim_check = true;
					g_DoubletapAimCheck = false;
				}

				//auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

				if (!double_tap_aim_check && cmd->buttons & IN_ATTACK && pWeapon->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER)
					g_DoubletapAim = false;
			}*/

		}
		prediction->EndPrediction();



		if (g_Options.edgejump.enabled && GetAsyncKeyState(g_Options.edgejump.hotkey))
		{
			if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && (flags & FL_ONGROUND))
				cmd->buttons |= IN_JUMP;

			if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && g_Options.edgejump.edge_jump_duck_in_air && !(cmd->buttons |= IN_DUCK))
				cmd->buttons |= IN_DUCK;
		}

		g_Options.extended_bt ? CLagCompensation::Get().UpdateIncomingSequences(pNetChannel) : CLagCompensation::Get().ClearIncomingSequences();
		if (g_Options.misc_showranks && cmd->buttons & IN_SCORE) g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);
		g_CVar->FindVar("weapon_debug_spread_show")->SetValue(g_Options.sniper_xhair && !g_LocalPlayer->m_bIsScoped() ? 3 : 0);

		Math::FixAngles(cmd->viewangles);
		Math::CorrectMovement(cmd, oldAngle, cmd->viewangles);

		return false;
	}

	bool __fastcall hkIsHLTV(uintptr_t ecx, uintptr_t edx) {
		static const auto AccLayers_Call = (void*)Utils::FindPattern2("client.dll", "84 C0 75 0D F6 87");
		static const auto SetupVelocity_Call = (void*)Utils::FindPattern2("client.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0");


		static auto oIsHLTV = enginemode_hook.get_original<decltype(&hkIsHLTV)>(93);
		auto pLocal = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());

		if (!g_EngineClient->IsInGame() || !pLocal)
			return oIsHLTV(ecx, edx);

		if (_ReturnAddress() == AccLayers_Call || _ReturnAddress() == SetupVelocity_Call)
			return true;

		return oIsHLTV(ecx, edx);
	}

	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}

	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				gDrawManager.Initialize();
				panelId = panel;
			}
		}
		else if (panelId == panel) 
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			int w, h;

			//god this code is a emss im sorry
			switch (g_Options.misc_watermark)
			{
			case 1:
			{
				std::string stext = Security::HWIDMatch() == 2 ? "CYBERWARE [DEV] | " : "CYBERWARE | ";
				stext += "FPS: " + std::to_string(Utils::GetFPS());
				const char* text = stext.c_str();
				size_t original_size = strlen(text) + 1;
				const size_t new_size = 1024;
				size_t converted_characters = 0;
				wchar_t wcstring[new_size];
				mbstowcs_s(&converted_characters, wcstring, original_size, text, _TRUNCATE);
				g_VGuiSurface->GetTextSize(Font::Get().MenuText, wcstring, w, h);
				int size_text = w + 50, offset = 10;
				gDrawManager.DrawRect(offset + 5, 5, size_text - 20, h + 15, Color(27, 27, 27, 150));
				gDrawManager.DrawRect(offset + 5, 10, size_text - 20, h + 10, Color(27, 27, 27, 255));
				gDrawManager.DrawString(size_text / 2 + 5, h, Color(255, 255, 255, 255), Font::Get().MenuText, text, AlignmentCenter);
				break;
			}
			case 2:
			{
				std::string stext = Security::HWIDMatch() == 2 ? "cyberware [dev] | " : "cyberware | ";
				stext += "fps: " + std::to_string(Utils::GetFPS());
				const char* text = stext.c_str();
				size_t original_size = strlen(text) + 1;
				const size_t new_size = 1024;
				size_t converted_characters = 0;
				wchar_t wcstring[new_size];
				mbstowcs_s(&converted_characters, wcstring, original_size, text, _TRUNCATE);
				g_VGuiSurface->GetTextSize(Font::Get().MenuText, wcstring, w, h);
				int size_text = w + 50;
				gDrawManager.DrawString(size_text / 2 + 5, h, Color(255, 255, 255, 255), Font::Get().MenuText, text, AlignmentCenter);
				break;
			}
			default:
				break;
			}
			g_CurrentMap = g_EngineClient->GetLevelName();
			Visuals::Get().Render();
			Render::Get().BeginScene();
			if (!g_EngineClient->IsInGame())
			{
				if ((g_Options.misc_discordrpc && g_DiscordShutdown) || (!g_Options.misc_discordrpc && !g_DiscordShutdown))
					CDiscordRPC::Get().Update();
			}
			/*if (g_Options.Desync && g_EngineClient->IsInGame() && g_LocalPlayer) {
				int wa, ha;
				g_EngineClient->GetScreenSize(wa, ha);
				float x = wa * 0.5f;
				float y = ha * 0.5f;
				gDrawManager.DrawString(CMisc::side > 0.0f ? x - 120 : x + 120, y - 0.f, Color::White, Font::Get().Velocity, CMisc::side < 0.0f ? ">>>" : " <<<");
			}*/

		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (g_Options.autoaccept && !strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && (g_Options.glow_enabled || g_Options.glow_weapons || g_Options.glow_planted_c4))
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		if (!g_EngineClient->IsInGame())
		{
			CLagCompensation::Get().ClearIncomingSequences();
			return 	ofunc(g_CHLClient, edx, stage);
		}

		if (!g_LocalPlayer)
			return 	ofunc(g_CHLClient, edx, stage);

		static auto BacktrackInit = (CLagCompensation::Get().Init(), false);
		skins::PlayerModel(stage);
		switch (stage)
		{
			case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
			{
				CDiscordRPC::Get().Update();
				break;
			}
			case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			{
				skins::on_frame_stage_notify(true);
				break;
			}
			case FRAME_RENDER_START:
			{
			//	CLagCompensation::Get().Update();
				break;
			}
		}
		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}


		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}

	void __fastcall RPEHk(void* client_state, void* edx, void* ent_info) {
		static auto oRPE = rpe_hook.get_original<rpe_fn>(64);
		oRPE(client_state, ent_info);
		skins::on_frame_stage_notify(false);
	}

	void RecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto ofunc = sequence_hook->get_original_function();

		if (g_LocalPlayer && g_LocalPlayer->IsAlive()) {
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);

			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid()) {
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer())) {
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid()) {
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon) {
							if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex())) {
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = skins::GetNewAnimation(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}

		}

		ofunc(pData, entity, output);
	}

	void __fastcall hkDrawModelExecute2(void* _this, int, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
	{
		static auto ofunc = stdrender_hook.get_original<decltype(&hkDrawModelExecute2)>(index::DrawModelExecute2);

		if (g_StudioRender->IsForcedMaterialOverride())
			return ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		static auto flash = g_MatSystem->FindMaterial("effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
		static auto flash_white = g_MatSystem->FindMaterial("effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);
		flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		flash_white->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		std::vector<const char*> vistasmoke_mats =

		{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		for (auto mat_s : vistasmoke_mats)
		{
			IMaterial* mat = g_MatSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, g_Options.no_smoke);
		}

		Chams::Get().OnDrawModelExecute(pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		g_StudioRender->ForcedMaterialOverride(nullptr);
	}

	void __fastcall Hooks::RenderSmokeOverlay(void* _this, int, const bool unk)
	{
		static auto ofunc = viewrender_hook.get_original<decltype(&RenderSmokeOverlay)>(index::RenderSmokeOverlay);

		if (!g_Options.no_smoke)
			ofunc(g_ViewRender, 0, unk);
	}
}
