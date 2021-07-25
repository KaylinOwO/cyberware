#pragma once
#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"

namespace GrenadePrediction {
	void predict(CUserCmd* ucmd);
	bool detonated(C_BaseCombatWeapon* weapon, float time, trace_t& trace);
	void trace(CUserCmd* ucmd);
	void draw();
}
