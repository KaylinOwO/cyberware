#pragma once

#include "../../singleton.hpp"

#include "../../render.hpp"
#include "../../helpers/math.hpp"
#include "../../valve_sdk/csgostructs.hpp"

struct SoundInfo_t {
	int guid;
	float soundTime;
	float alpha;
	Vector soundPos;
};

struct SBoundingBox {
	SBoundingBox() {
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
	}

	SBoundingBox(int x, int y, int w, int h) {
		this->x = x;
		this->y = x;
		this->w = w;
		this->h = h;
	}
	int x, y, w, h;
};

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;

		bool Begin(C_BaseEntity* pl);
		void RenderBox(C_BaseEntity* pl);
		void RenderName(C_BaseEntity* pl);
		void RenderWeaponName(C_BaseEntity* pl);
		void RenderHealth(C_BaseEntity* pl);
		void RenderArmour();
		void RenderSnapline();
	};

	CUtlVector<SndInfo_t> sounds;

	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void RenderSounds();
	void UpdateSounds();
//*Surface*//
public:
	std::deque<HitMarkerObject_t> vecHitMarks = { };
	bool GetBoundingBoxPositions(C_BasePlayer* player, SBoundingBox& bounding_box) {
		SBoundingBox output{ };

		Vector trans = player->abs_origin();

		Vector min;
		Vector max;

		auto collideble = player->GetCollideable();
		min = collideble->OBBMins();
		max = collideble->OBBMaxs();

		Vector point_list[] = {
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector transformed[8];

		for (int i = 0; i < 8; i++) {
			transformed[i].x = point_list[i].x + trans.x;
			transformed[i].y = point_list[i].y + trans.y;
			transformed[i].z = point_list[i].z + trans.z;
		}

		Vector flb, brt{}, blb{}, frt{}, frb{}, brb{}, blt{}, flt{};

		if (g_DebugOverlay->ScreenPosition(transformed[3], flb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[0], blb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[2], frb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[6], blt) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[5], brt) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[4], frt) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[1], brb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[7], flt) == 1)
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

		float left = flb.x;
		float top = flb.y;
		float right = flb.x;
		float bottom = flb.y;

		for (int i = 0; i < 8; i++) {
			if (left > arr[i].x)
				left = arr[i].x;
			if (top < arr[i].y)
				top = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (bottom > arr[i].y)
				bottom = arr[i].y;
		}

		bounding_box.x = left;
		bounding_box.y = bottom;
		bounding_box.w = right - left;
		bounding_box.h = top - bottom;
		return true;
	}
	bool GetBoundingBoxPositions(C_BaseEntity* player, SBoundingBox& bounding_box) {
		SBoundingBox output{ };

		Vector trans = player->m_vecOrigin();

		Vector min;
		Vector max;

		auto collideble = player->GetCollideable();
		min = collideble->OBBMins();
		max = collideble->OBBMaxs();

		Vector point_list[] = {
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector transformed[8];

		for (int i = 0; i < 8; i++) {
			transformed[i].x = point_list[i].x + trans.x;
			transformed[i].y = point_list[i].y + trans.y;
			transformed[i].z = point_list[i].z + trans.z;
		}

		Vector flb, brt{}, blb{}, frt{}, frb{}, brb{}, blt{}, flt{};

		if (g_DebugOverlay->ScreenPosition(transformed[3], flb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[0], blb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[2], frb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[6], blt) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[5], brt) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[4], frt) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[1], brb) == 1 ||
			g_DebugOverlay->ScreenPosition(transformed[7], flt) == 1)
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

		float left = flb.x;
		float top = flb.y;
		float right = flb.x;
		float bottom = flb.y;

		for (int i = 0; i < 8; i++) {
			if (left > arr[i].x)
				left = arr[i].x;
			if (top < arr[i].y)
				top = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (bottom > arr[i].y)
				bottom = arr[i].y;
		}

		bounding_box.x = left;
		bounding_box.y = bottom;
		bounding_box.w = right - left;
		bounding_box.h = top - bottom;
		return true;
	}
	void Render();
	void ESP(C_BasePlayer* e, SBoundingBox Box, bool EntityIsAlive = true);
	void ESP(C_BaseEntity* e, SBoundingBox Box, bool EntityIsAlive = true);
	void PlayerESP();
	void Hitmarker();
	void Velocity();
public:
	void AddToDrawList();
};
