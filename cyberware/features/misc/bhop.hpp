#pragma once

class C_BasePlayer;
class CUserCmd;

namespace BunnyHop
{
    void OnCreateMove(CUserCmd* cmd);
    void AutoStafe(CUserCmd* cmd);
    void Jumpbug(CUserCmd* cmd);
}