#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}
template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

struct legitbot_s
{
	bool enabled = false;
	bool deathmatch = false;
	//bool silent = false;
	bool silent2 = false;

	bool flash_check = false;
	bool smoke_check = false;
	bool autopistol = false;

	float fov = 0.f;
	float silent_fov = 0.f;
	float smooth = 1.f;

	int shot_delay = 0;
	int kill_delay = 0;

	struct
	{
		bool head = true;
		bool chest = true;
		bool hands = true;
		bool legs = true;
	} hitboxes;

	struct
	{
		bool enabled = false;
		int start = 1;
		int type = 0;
		int x = 100;
		int y = 100;
	} rcs;

	struct
	{
		bool enabled = false;
		int min_damage = 1;
	} autowall;

	struct
	{
		bool enabled = false;
		int hotkey = 0;
	} autofire;

};

struct weapons
{
	legitbot_s legit;
};
struct statrack_setting
{
	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};
struct item_setting
{
	char name[32] = "Default";
	//bool enabled = false;
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};
class Options
{
public:
	std::map<short, weapons> weapons;
	struct
	{
		/*struct
		{
			std::map<int, profilechanger_settings> profile_items = { };
		}profile;*/
		struct
		{
			bool skin_preview = false;
			bool show_cur = true;

			std::map<int, statrack_setting> statrack_items = { };
			std::map<int, item_setting> m_items = { };
			std::map<std::string, std::string> m_icon_overrides = { };
		}skin;
	}changers;
		//
		// AIMBOT
		//
		int aim_weaponselect = 0;
		bool aim_enabled;
		bool aim_silent_pistol, aim_silent_rifle, aim_silent_sniper, aim_silent_scout, aim_silent_smg, aim_silent_heavy;

		bool aim_flash_check = false;
		bool aim_smoke_check = false;
		bool aim_autopistol = false;

		float aim_fov_pistol, aim_fov_rifle, aim_fov_sniper, aim_fov_scout, aim_fov_smg, aim_fov_heavy = 0.f;
		float aim_smooth_pistol, aim_smooth_rifle, aim_smooth_sniper, aim_smooth_scout, aim_smooth_smg, aim_smooth_heavy = 1.f;

		int aim_shotdelay_rifle, aim_shotdelay_smg, aim_shotdelay_heavy = 0;

		int aim_type_pistol = 1, aim_type_rifle = 1, aim_type_sniper = 1, aim_type_scout = 1, aim_type_smg = 1, aim_type_heavy = 1;
		int aim_hitbox_pistol = 0, aim_hitbox_rifle = 0, aim_hitbox_sniper = 0, aim_hitbox_scout = 0, aim_hitbox_smg = 0, aim_hitbox_heavy = 0;

		bool aim_rcs_pistol, aim_rcs_rifle, aim_rcs_smg, aim_rcs_heavy = false, aim_rcs_sniper = false;
		int aim_rcs_start_pistol = 1, aim_rcs_start_rifle = 1, aim_rcs_start_smg = 1, aim_rcs_start_heavy = 1, aim_rcs_start_sniper = 1;
		int aim_rcs_type_pistol, aim_rcs_type_rifle, aim_rcs_type_smg, aim_rcs_type_heavy, aim_rcs_type_sniper;
		int aim_rcs_x_pistol = 100, aim_rcs_x_rifle = 100, aim_rcs_x_smg = 100, aim_rcs_x_heavy = 100, aim_rcs_x_sniper = 100;
		int aim_rcs_y_pistol = 100, aim_rcs_y_rifle = 100, aim_rcs_y_smg = 100, aim_rcs_y_heavy = 100, aim_rcs_y_sniper = 100;

		bool aim_autowall_pistol, aim_autowall_rifle, aim_autowall_sniper, aim_autowall_scout, aim_autowall_smg, aim_autowall_heavy;
		int aim_mindamage_pistol, aim_mindamage_rifle, aim_mindamage_sniper, aim_mindamage_scout, aim_mindamage_smg, aim_mindamage_heavy = 1;

		// TRIGGER
		bool trigger_enabled = false;
		int trigger_key, trigger_delay;
		bool trigger_head, trigger_chest, trigger_stomach, trigger_arms, trigger_legs;
		
	
		// 
		// ESP
		// 
		bool esp_enabled = false;
		bool esp_visonly = false;
		bool esp_player_boxes = false;
		bool esp_player_names = false;
		bool esp_player_health = false;
		bool esp_player_armour = false;
		bool esp_player_weapons = false;
		bool esp_player_snaplines = false;
		bool esp_dropped_weapons = false;
		bool esp_defuse_kit = false;
		bool esp_planted_c4 = false;
		bool esp_items = false;

		// 
		// GLOW
		// 
		bool glow_enabled = false;
		bool glow_enemies_only = false;
		bool glow_players = false;
		bool glow_planted_c4 = false;
		bool glow_defuse_kits = false;
		bool glow_weapons = false;
		bool glow_enemiesOC = false;

		//
		// CHAMS
		//
		bool chams_player_enabled = false;
		bool player_enemies_shine = false;
		bool chams_player_backtrack = false, chams_player_backtrack_allticks;
		int chams_player_backtrack_type = 0;

		bool chams_player_enemies_only = false;
		bool chams_player_wireframe = false;
		int chams_player_flat = false;
		bool chams_player_ignorez = false;
		bool chams_player_glass = false;
		bool chams_arms_enabled = false;
		bool chams_arms_wireframe = false;
		bool chams_arms_flat = false;
		bool chams_arms_ignorez = false;
		bool chams_arms_glass = false;

		//
		// MISC
		//
		bool misc_bhop = false;
		bool misc_bhop2 = false;
		bool misc_clantag = false;
		bool misc_discordrpc = false;
		int misc_clantag_type = 0;
		char misc_clantag_string[64];
		int playerModelT{ 0 };
		int playerModelCT{ 0 };

		bool MDoubletap, MHideShots, Doubletap, HideShots, DoubletapKey, HideShotsKey, SlowTeleport;
		int MDoubletapKey, MHideShotsKey;

		bool Desync; int DesyncType, DesyncSwitchKey;
		bool hitmarker, killsound, hitsound, damage_indicator;


		bool misc_showranks = false;
		int misc_watermark = 1;
		bool Velocity = false;
		bool LastJump = false;
		bool Graph = false;
		bool autoaccept = false;
		bool world_modulation;
		bool no_flash = false;
		bool no_smoke = false;
		bool spectator_list = false;
		int AutoStafe_key;
		bool autostrafe = false;
		bool misc_bt = false, extended_bt = false; int bt_ticks = 200;
		struct
		{
			bool enabled = false;
			bool edge_jump_duck_in_air = false;

			int hotkey = 0;
		} edgejump;
		bool edge_bug;
		int edge_bug_key;
		bool jump_bug = false;
		int jump_bug_key;
		int glow_enemies_type, glow_enemies_occluded_type;
		bool sniper_xhair = false;
		bool GrenadeIndicator = false;
		// 
		// COLORS
		// 
		Color color_esp_ally = { 0, 0, 0 };
		Color color_esp_enemy = { 255, 0, 0 };
		Color color_esp_weapons= {0, 0, 0};
		Color color_esp_defuse= {0, 0, 0};
		Color color_esp_c4= {0, 0, 0};
		Color color_esp_item= {0, 0, 0};
		Color Velocitycol = { 255, 255, 255 };

		Color color_glow_ally= {0, 0, 0};
		Color color_glow_allyOC= {0, 0, 0};

		Color color_glow_enemy= {255, 0, 0};
		Color color_glow_enemyOC= {255, 0, 0};

		Color color_glow_chickens= {0, 0, 0};
		Color color_glow_c4_carrier= {0, 0, 0};
		Color color_glow_planted_c4= {0, 0, 0};
		Color color_glow_defuse= {0, 0, 0};
		Color color_glow_weapons= {0, 0, 0};

		Color color_chams_player_ally_visible= {0, 0, 0};
		Color color_chams_player_ally_occluded= {0, 0, 0};
		Color color_chams_player_enemy_visible= {255, 0, 0};
		Color color_chams_player_enemy_occluded= {255, 0, 0};
		Color color_chams_arms_visible= {0, 0, 0};
		Color color_chams_arms_occluded= {0, 0, 0};
		Color color_chams_backtrack = { 255, 255, 255 };
		Color player_enemy_visible_shine = { 0, 0, 0 };

		Color color_grenade_indicator = { 0, 0, 0 };
		Color color_hitmarker = { 0, 0, 0 };
		Color color_damageindicator = { 0, 0, 0 };
		Color color_worldmoulation = { 0, 0, 0 };

		// 
		// CHEAT
		//

protected:
	//std::vector<ConfigValue<char>*> chars;
	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<char>*> chars;
	std::vector<ConfigValue<float>*> floats;
private:
//	void SetupValue(char value, std::string category, std::string name);
	void SetupValue(int& value, std::string category, std::string name);
	void SetupValue(bool& value, std::string category, std::string name);
	void SetupValue(char& value, std::string category, std::string name);
	void SetupValue(float& value, std::string category, std::string name);
	void SetupColor(Color& value, const std::string& name);
	void SetupWeapons();
	void SetupVisuals();
	void SetupMisc();
	void SetupColors();
public:
	void Initialize();
	void LoadSettings(const std::string& szIniFile);
	void SaveSettings(const std::string& szIniFile);
	void DeleteSettings(const std::string& szIniFile);

	std::string folder;
};

inline Options g_Options;
inline bool   g_Unload;
