#pragma once
#include "../../valve_sdk/sdk.hpp"
#include "../..//valve_sdk/csgostructs.hpp"

class C_BasePlayer;
class CUserCmd;

namespace CMisc
{
    void Choke(bool* bSendPacket);
    void Desync(CUserCmd* cmd, bool* bSendPacket);
    void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles);
    void CorrectMouse(CUserCmd* pCmd);
    void AntiTrigger(CUserCmd* cmd);
    void AntiTriggerchoke(bool* bSendPacket, bool* bOldPacket);

    inline float next_lby = 0.0f;
    inline float side = 1.0f;
}