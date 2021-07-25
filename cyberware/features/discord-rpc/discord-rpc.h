#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/misc/discord/include/discord_rpc.h"

class CDiscordRPC : public Singleton<CDiscordRPC>
{
public:
	// Get
	void Update();
	void Initialize();
};