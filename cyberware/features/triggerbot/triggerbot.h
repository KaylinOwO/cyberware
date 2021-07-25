#pragma once
#include <chrono>
#include "../..//options.hpp"
#include "../..//valve_sdk/csgostructs.hpp"

class CTimer
{
private:
	std::chrono::high_resolution_clock::time_point timePoint = { };
public:
	CTimer(bool bStart = false)
	{
		if (bStart)
			Reset();
	}

	/* set time point to now */
	void Reset()
	{
		timePoint = std::chrono::high_resolution_clock::now();
	}

	/* returns elapsed time between last time point and now in given duration type (default: milliseconds) */
	template <class C = std::chrono::milliseconds>
	long long Elapsed() const
	{
		return std::chrono::duration_cast<C>(std::chrono::high_resolution_clock::now() - timePoint).count();
	}
};

class CTriggerBot
{
public:
	void Run(CUserCmd* cmd);
private:
	CTimer timer = { };
};

inline CTriggerBot* g_Triggerbot;