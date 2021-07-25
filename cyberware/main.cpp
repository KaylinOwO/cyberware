#define NOMINMAX
#include <Windows.h>
#include "helpers/security/security.h"
#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include "features/aimbot/aimbot.hpp"
#include "features/kit_parser.h"
#include "main.h"
#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"

#include "valve_sdk/gameoverlayhooking/pe/pe.h"
#include "valve_sdk/gameoverlayhooking/pe/util/util.h"
#include "valve_sdk/gameoverlayhooking/gameoverlayhooking.h"

#include "features/discord-rpc/discord-rpc.h"


DWORD WINAPI OnDllAttach(LPVOID base)
{
	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(1000);

#ifdef _DEBUG
    Utils::AttachConsole();
#endif

    try {
      //  cleanup();

        Utils::ConsolePrint("Initializing...\n");
        Interfaces::Initialize();
        Interfaces::Dump();

        Font::Get().Create();
        NetvarSys::Get().Initialize();
        InputSys::Get().Initialize();
        Render::Get().Initialize();
        Menu::Get().Initialize();
        g_Legitbot = new CLegitbot();
        g_Options.Initialize();
    
        g_Options.LoadSettings("default.ini");
            
        initialize_kits();

        Hooks::Initialize();
        CDiscordRPC::Get().Initialize();
        Utils::EventListener.Setup({ "player_death", "player_hurt", "round_prestart", "round_freeze_end", "round_start", "player_spawned" });

        if (!pe::init::the())   printf("oh fuck pe no!!!");
        if (!GOHooking::Functions::Initialize::the())   printf("oh fuck color modulation no!!!");

        // Menu Toggle
        InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
            Menu::Get().Toggle();
            });

        Utils::ConsolePrint("Finished.\n");
		Utils::ConsolePrint("Built on: %s %s\n", __DATE__, __TIME__);

        while(!g_Unload)
            Sleep(1000);

        g_CVar->FindVar("crosshair")->SetValue(true);

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);

     //   cleanup();
    } catch(const std::exception& ex) {
        Utils::ConsolePrint("An error occured during initialization:\n");
        Utils::ConsolePrint("%s\n", ex.what());
        Utils::ConsolePrint("Press any key to exit.\n");
        Utils::ConsoleReadKey();
        Utils::DetachConsole();

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    }

    // unreachable
    //return TRUE;
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Utils::DetachConsole();
#endif

    Hooks::Shutdown();
    delete g_Legitbot;

    Menu::Get().Shutdown();
    Utils::EventListener.Destroy();
    return TRUE;
}

BOOL WINAPI DllMain(
    _In_      HINSTANCE hinstDll,
    _In_      DWORD     fdwReason,
    _In_opt_  LPVOID    lpvReserved
)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
         //   cleanup();
            DisableThreadLibraryCalls(hinstDll);
            if (Security::Authentication())
                CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
            return TRUE;
        case DLL_PROCESS_DETACH:
            if(lpvReserved == nullptr)
                return OnDllDetach();
            return TRUE;
        default:
            return TRUE;
    }
}
