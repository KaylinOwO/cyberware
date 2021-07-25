#pragma once
#include <Windows.h>
#include "include/discord_register.h"
#include "include/discord_rpc.h"

namespace DiscordHandles {
	//from https://developer.valvesoftware.com/wiki/Implementing_Discord_RPC
	static void Discord_Ready(const DiscordUser* connectedUser)
	{
		printf("Discord poppin");
	}

	static void Discord_Disconnect(int errcode, const char* message)
	{
		printf("Discord not poppin");
	}

	static void Discord_Error(int errcode, const char* message)
	{
		printf("Discord err0r");
	}
}