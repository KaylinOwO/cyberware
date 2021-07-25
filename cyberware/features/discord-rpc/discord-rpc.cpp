#include "discord-rpc.h"
#include "../../valve_sdk/misc/discord/discord_handles.h"
#include "../../options.hpp"


void CDiscordRPC::Update()
{
	if (!g_Options.misc_discordrpc && !g_DiscordShutdown)
		Discord_Shutdown();
	else if (g_Options.misc_discordrpc && g_DiscordShutdown)
	{
		DiscordEventHandlers EventHandlers;
		memset(&EventHandlers, 0, sizeof(EventHandlers));

		EventHandlers.ready = DiscordHandles::Discord_Ready;
		EventHandlers.disconnected = DiscordHandles::Discord_Disconnect;
		EventHandlers.errored = DiscordHandles::Discord_Error;
		Discord_Initialize("0", &EventHandlers, 1, "730");
	}


	if (g_Options.misc_discordrpc)
	{
		DiscordRichPresence DiscordPresence;
		memset(&DiscordPresence, 0, sizeof(DiscordPresence));
		DiscordPresence.details = "gaming with cyberware";
		std::string map = "playing on ";
		map += g_CurrentMap;
		DiscordPresence.state = g_EngineClient->IsInGame() && !g_EngineClient->IsDrawingLoadingImage() ? map.c_str() : "chilling @ main menu";
		DiscordPresence.largeImageKey = "0";
		Discord_UpdatePresence(&DiscordPresence);
	}
}

void CDiscordRPC::Initialize()
{
	DiscordEventHandlers EventHandlers;
	memset(&EventHandlers, 0, sizeof(EventHandlers));

	EventHandlers.ready = DiscordHandles::Discord_Ready;
	EventHandlers.disconnected = DiscordHandles::Discord_Disconnect;
	EventHandlers.errored = DiscordHandles::Discord_Error;
	Discord_Initialize("0", &EventHandlers, 1, "730");
	Update();
}