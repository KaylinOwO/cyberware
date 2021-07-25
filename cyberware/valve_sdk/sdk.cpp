#include "sdk.hpp"

#include "../Helpers/Utils.hpp"
#include "csgostructs.hpp"
template <typename T>
static constexpr auto RelativeToAbsolute(uintptr_t address) noexcept
{
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

namespace Interfaces
{
    CreateInterfaceFn get_module_factory(HMODULE module)
    {
        return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, "CreateInterface"));
    }

    template<typename T>
    T* get_interface(CreateInterfaceFn f, const char* szInterfaceVersion)
    {
        auto result = reinterpret_cast<T*>(f(szInterfaceVersion, nullptr));

        if(!result) {
            throw std::runtime_error(std::string("[get_interface] Failed to GetOffset interface: ") + szInterfaceVersion);
        }

        return result;
    }

    void Initialize()
    {
        auto engineFactory      = get_module_factory(GetModuleHandleW(L"engine.dll"));
        auto clientFactory      = get_module_factory(GetModuleHandleW(L"client.dll"));
        auto valveStdFactory    = get_module_factory(GetModuleHandleW(L"vstdlib.dll"));
        auto vguiFactory        = get_module_factory(GetModuleHandleW(L"vguimatsurface.dll"));
        auto vgui2Factory       = get_module_factory(GetModuleHandleW(L"vgui2.dll"));
        auto matSysFactory      = get_module_factory(GetModuleHandleW(L"materialsystem.dll"));
        auto dataCacheFactory   = get_module_factory(GetModuleHandleW(L"datacache.dll"));
        auto vphysicsFactory    = get_module_factory(GetModuleHandleW(L"vphysics.dll"));
        auto inputSysFactory    = get_module_factory(GetModuleHandleW(L"inputsystem.dll"));
        auto matchmakingFactory = get_module_factory(GetModuleHandleW(L"matchmaking.dll"));
        auto studioFactory      = get_module_factory(GetModuleHandleW(L"studiorender.dll"));

        g_CHLClient           = get_interface<IBaseClientDLL>      (clientFactory     , "VClient018");
        g_EntityList          = get_interface<IClientEntityList>   (clientFactory     , "VClientEntityList003");
        g_Prediction          = get_interface<IPrediction>         (clientFactory     , "VClientPrediction001");
        g_GameMovement        = get_interface<CGameMovement>       (clientFactory     , "GameMovement001");
        g_MdlCache            = get_interface<IMDLCache>           (dataCacheFactory  , "MDLCache004");
        g_EngineClient        = get_interface<IVEngineClient>      (engineFactory     , "VEngineClient014");
        g_MdlInfo             = get_interface<IVModelInfoClient>   (engineFactory     , "VModelInfoClient004");
        g_MdlRender           = get_interface<IVModelRender>       (engineFactory     , "VEngineModel016");
        g_RenderView          = get_interface<IVRenderView>        (engineFactory     , "VEngineRenderView014");
        g_EngineTrace         = get_interface<IEngineTrace>        (engineFactory     , "EngineTraceClient004");
        g_DebugOverlay        = get_interface<IVDebugOverlay>      (engineFactory     , "VDebugOverlay004");
        g_GameEvents          = get_interface<IGameEventManager2>  (engineFactory     , "GAMEEVENTSMANAGER002");
        g_EngineSound         = get_interface<IEngineSound>        (engineFactory     , "IEngineSoundClient003");
        g_MatSystem           = get_interface<IMaterialSystem>     (matSysFactory     , "VMaterialSystem080");
        g_CVar                = get_interface<ICvar>               (valveStdFactory   , "VEngineCvar007");
        g_VGuiPanel           = get_interface<IPanel>              (vgui2Factory      , "VGUI_Panel009");
        g_VGuiSurface         = get_interface<ISurface>            (vguiFactory       , "VGUI_Surface031");
        g_PhysSurface         = get_interface<IPhysicsSurfaceProps>(vphysicsFactory   , "VPhysicsSurfaceProps001");
        g_InputSystem         = get_interface<IInputSystem>        (inputSysFactory   , "InputSystemVersion001");
        g_GameTypes           = get_interface<IGameTypes>          (matchmakingFactory, "VENGINE_GAMETYPES_VERSION002");
        g_StudioRender        = get_interface<IStudioRender>       (studioFactory     ,  "VStudioRender026");

        auto client = GetModuleHandleW(L"client.dll");
        auto engine = GetModuleHandleW(L"engine.dll");
        auto dx9api = GetModuleHandleW(L"shaderapidx9.dll");
		do {
			g_ClientMode  =      **(IClientMode***)((*(uintptr_t**)g_CHLClient)[10] + 0x5);
		} while (!g_ClientMode);
        g_GlobalVars      =  **(CGlobalVarsBase***)(Utils::PatternScan(client, "A1 ? ? ? ? 5E 8B 40 10") + 1);
		g_Input           =             *(CInput**)(Utils::PatternScan(client, "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 1);
		g_MoveHelper      =      **(IMoveHelper***)(Utils::PatternScan(client, XorStr("8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01").c_str()) + 2);
		g_GlowObjManager  = *(GlowObjectManager**)(Utils::PatternScan(client, XorStr("0F 11 05 ? ? ? ? 83 C8 01").c_str()) + 3);
		g_ViewRender      =        *(IViewRender**)(Utils::PatternScan(client, "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10") + 1);
		g_D3DDevice9      = **(IDirect3DDevice9***)(Utils::PatternScan(dx9api, "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
		g_ClientState     =     **(CClientState***)(Utils::PatternScan(engine, XorStr("A1 ? ? ? ? 8B 80 ? ? ? ? C3").c_str()) + 1);
		g_LocalPlayer     =       *(C_LocalPlayer*)(Utils::PatternScan(client, "8B 0D ? ? ? ? 83 FF FF 74 07") + 2);
		g_WeaponSystem    = *(IWeaponSystem * *)(Utils::PatternScan(client, "8B 35 ? ? ? ? FF 10 0F B7 C0") + 2);
        g_RenderBeams = *(IViewRenderBeams**)(Utils::PatternScan(client, "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 1);
        g_GameRules = *(CGameRules**)(Utils::PatternScan(client, "8B 0D ?? ?? ?? ?? 85 C0 74 0A 8B 01 FF 50 78 83 C0 54") + 2);
        g_SetAbsOrigin = RelativeToAbsolute<decltype(g_SetAbsOrigin)>(Utils::FindPattern("client.dll", "\xE8????\xEB\x19\x8B\x07") + 1);
    }

    void Dump()
    {
        // Ugly macros ugh
        #define STRINGIFY_IMPL(s) #s
        #define STRINGIFY(s)      STRINGIFY_IMPL(s)
        #define PRINT_INTERFACE(name) Utils::ConsolePrint("%-20s: %p\n", STRINGIFY(name), name)

        PRINT_INTERFACE(g_CHLClient   );
        PRINT_INTERFACE(g_EntityList  );
        PRINT_INTERFACE(g_Prediction  );
        PRINT_INTERFACE(g_GameMovement);
        PRINT_INTERFACE(g_MdlCache    );
        PRINT_INTERFACE(g_EngineClient);
        PRINT_INTERFACE(g_MdlInfo     );
        PRINT_INTERFACE(g_MdlRender   );
        PRINT_INTERFACE(g_RenderView  );
        PRINT_INTERFACE(g_EngineTrace );
        PRINT_INTERFACE(g_DebugOverlay);
        PRINT_INTERFACE(g_GameEvents  );
        PRINT_INTERFACE(g_EngineSound );
        PRINT_INTERFACE(g_MatSystem   );
        PRINT_INTERFACE(g_CVar        );
        PRINT_INTERFACE(g_VGuiPanel   );
        PRINT_INTERFACE(g_VGuiSurface );
        PRINT_INTERFACE(g_PhysSurface );
        PRINT_INTERFACE(g_InputSystem );
        PRINT_INTERFACE(g_GameTypes   );
        PRINT_INTERFACE(g_StudioRender);

    }
}
