#include <ShlObj.h>
#include <filesystem>
#include "options.hpp"
#include "valve_sdk/misc/Enums.hpp"
#include "valve_sdk/sdk.hpp"

const std::map<int, const char*> config_names = {
	{WEAPON_CZ75A, "CZ75 Auto"},
	{WEAPON_DEAGLE, "Desert Eagle"},
	{WEAPON_ELITE, "Dual Berettas"},
	{WEAPON_FIVESEVEN, "Five-SeveN"},
	{WEAPON_HKP2000, "P2000"},
	{WEAPON_P250, "P250"},
	{WEAPON_USP_SILENCER, "USP-S"},
	{WEAPON_TEC9, "Tec-9"},
	{WEAPON_REVOLVER, "R8 Revolver"},
	{WEAPON_GLOCK, "Glock-18"},

	{WEAPON_MAG7, "MAG-7"},
	{WEAPON_NOVA, "Nova"},
	{WEAPON_SAWEDOFF, "Sawed-Off"},
	{WEAPON_XM1014, "XM1014"},

	{WEAPON_UMP45, "UMP-45"},
	{WEAPON_P90, "P90"},
	{WEAPON_BIZON, "PP-Bizon"},
	{WEAPON_MAC10, "MAC-10"},
	{WEAPON_MP7, "MP7"},
	{WEAPON_MP9, "MP9"},
	{WEAPON_MP5, "MP5-SD"},

	{WEAPON_M249, "M249"},
	{WEAPON_NEGEV, "Negev"},

	{WEAPON_AK47, "AK-47"},
	{WEAPON_AUG, "AUG"},
	{WEAPON_GALILAR, "Galil AR"},
	{WEAPON_M4A1_SILENCER, "M4A1-S"},
	{WEAPON_M4A1, "M4A4"},
	{WEAPON_SG556, "SG 553"},
	{WEAPON_FAMAS, "FAMAS"},

	{WEAPON_AWP, "AWP"},
	{WEAPON_G3SG1, "G3SG1"},
	{WEAPON_SCAR20, "SCAR-20"},
	{WEAPON_SSG08, "SSG 08"},

	{WEAPON_KNIFE, "Knife"},
	{WEAPON_KNIFE_T, "TKnife"},

	{GLOVE_T_SIDE, "Glove"},
};

const std::map<int, const char*> k_weapon_names = {
	{WEAPON_KNIFE, "knife ct"},
	{WEAPON_KNIFE_T, "knife t"},
	{GLOVE_CT_SIDE, "glove ct"},
	{GLOVE_T_SIDE, "glove t"},
	{61, "usp"},
	{32, "p2000"},
	{4, "glock-18"},
	{2, "dual berettas"},
	{36, "p250"},
	{3, "five-seven"},
	{30, "tec-9"},
	{63, "cz75 auto"},
	{64, "r8 revolver"},
	{1, "deagle"},

	{34, "mp9"},
	{17, "mac-10"},
	{23, "mp5-sd"},
	{33, "mp7"},
	{24, "ump-45"},
	{19, "p90"},
	{26, "pp-bizon"},

	{7, "ak-47"},
	{60, "m4a1-s"},
	{16, "m4a4"},
	{8, "aug"},
	{39, "sg553"},
	{10, "famas"},
	{13, "galil"},

	{40, "ssg08"},
	{38, "scar-20"},
	{9, "awp"},
	{11, "g3sg1"},

	{14, "m249"},
	{28, "negev"},

	{27, "mag-7"},
	{35, "nova"},
	{29, "sawed-off"},
	{25, "xm1014"},
};

void Options::SetupValue(int& value, std::string category, std::string name)
{
	ints.push_back(new ConfigValue<int>(std::move(category), std::move(name), &value));
}
/*void Options::SetupValue(char value, std::string category, std::string name)
{
	chars.push_back(new ConfigValue<char>(std::move(category), std::move(name), &value));
}*/

void Options::SetupValue(bool& value, std::string category, std::string name)
{
	bools.push_back(new ConfigValue<bool>(std::move(category), std::move(name), &value));
}

void Options::SetupValue(char& value, std::string category, std::string name)
{
	chars.push_back(new ConfigValue<char>(std::move(category), std::move(name), &value));
}

void Options::SetupValue(float& value, std::string category, std::string name)
{
	floats.push_back(new ConfigValue<float>(std::move(category), std::move(name), &value));
}

void Options::SetupColor(Color& value, const std::string& name)
{
	SetupValue(value._CColor[0], "Colors", name + "_r");
	SetupValue(value._CColor[1], "Colors", name + "_g");
	SetupValue(value._CColor[2], "Colors", name + "_b");
	SetupValue(value._CColor[3], "Colors", name + "_a");
}



void Options::SetupWeapons()
{
	for (auto& [key, val] : k_weapon_names) {
		auto& option = g_Options.changers.skin.m_items[key];
		SetupValue(option.definition_vector_index, val, "d_vec_index");
		SetupValue(option.definition_index, val, "d_index");
		SetupValue(option.paint_kit_vector_index, val, "pk_vec_index");
		SetupValue(option.paint_kit_index, val, "pk_index");
		SetupValue(option.definition_override_index, val, "do_index");
		SetupValue(option.definition_override_vector_index, val, "do_vec_index");
		//SetupValue(option.seed, val, "seed");
		//SetupValue(option.enable_stat_track, val, "stattrack");
		//SetupValue(option.stat_trak, val, "stat_trak_val");
		//SetupValue(option.wear, val, "wear");
	}

}

void Options::SetupVisuals()
{
	/*AIMBOT*/
	SetupValue(g_Options.aim_enabled, "Aimbot", "Enabled");
	SetupValue(g_Options.aim_flash_check, "Aimbot", "Flash Check");
	SetupValue(g_Options.aim_smoke_check, "Aimbot", "Smoke Check");
	SetupValue(g_Options.aim_autopistol, "Aimbot", "Auto Pistol");

	SetupValue(g_Options.aim_fov_pistol, "Aimbot", "FOV Pistol");
	SetupValue(g_Options.aim_smooth_pistol, "Aimbot", "Smooth Pistol");
	SetupValue(g_Options.aim_silent_pistol, "Aimbot", "Silent Pistol");
	SetupValue(g_Options.aim_rcs_pistol, "Aimbot", "RCS Pistol");
	SetupValue(g_Options.aim_rcs_start_pistol, "Aimbot", "RCS Start Pistol");
	SetupValue(g_Options.aim_rcs_type_pistol, "Aimbot", "RCS Type Pistol");
	SetupValue(g_Options.aim_rcs_x_pistol, "Aimbot", "RCS X Pistol");
	SetupValue(g_Options.aim_rcs_y_pistol, "Aimbot", "RCS Y Pistol");
	SetupValue(g_Options.aim_autowall_pistol, "Aimbot", "Autowall Pistol");
	SetupValue(g_Options.aim_mindamage_pistol, "Aimbot", "Min Damage Pistol");
	SetupValue(g_Options.aim_type_pistol, "Aimbot", "Aim Type Pistol");
	SetupValue(g_Options.aim_hitbox_pistol, "Aimbot", "Aim Hitbox Pistol");

	SetupValue(g_Options.aim_fov_rifle, "Aimbot", "FOV Rifle");
	SetupValue(g_Options.aim_smooth_rifle, "Aimbot", "Smooth Rifle");
	SetupValue(g_Options.aim_silent_rifle, "Aimbot", "Silent Rifle");
	SetupValue(g_Options.aim_rcs_rifle, "Aimbot", "RCS Rifle");
	SetupValue(g_Options.aim_rcs_start_rifle, "Aimbot", "RCS Start Rifle");
	SetupValue(g_Options.aim_rcs_type_rifle, "Aimbot", "RCS Type Rifle");
	SetupValue(g_Options.aim_rcs_x_rifle, "Aimbot", "RCS X Rifle");
	SetupValue(g_Options.aim_rcs_y_rifle, "Aimbot", "RCS Y Rifle");
	SetupValue(g_Options.aim_autowall_rifle, "Aimbot", "Autowall Rifle");
	SetupValue(g_Options.aim_mindamage_rifle, "Aimbot", "Min Damage Rifle");
	SetupValue(g_Options.aim_type_rifle, "Aimbot", "Aim Type Rifle");
	SetupValue(g_Options.aim_hitbox_rifle, "Aimbot", "Aim Hitbox Rifle");

	SetupValue(g_Options.aim_fov_sniper, "Aimbot", "FOV Sniper");
	SetupValue(g_Options.aim_smooth_sniper, "Aimbot", "Smooth Sniper");
	SetupValue(g_Options.aim_silent_sniper, "Aimbot", "Silent Sniper");
	SetupValue(g_Options.aim_rcs_sniper, "Aimbot", "RCS Sniper");
	SetupValue(g_Options.aim_rcs_start_sniper, "Aimbot", "RCS Start Sniper");
	SetupValue(g_Options.aim_rcs_type_sniper, "Aimbot", "RCS Type Sniper");
	SetupValue(g_Options.aim_rcs_x_sniper, "Aimbot", "RCS X Sniper");
	SetupValue(g_Options.aim_rcs_y_sniper, "Aimbot", "RCS Y Sniper");
	SetupValue(g_Options.aim_autowall_sniper, "Aimbot", "Autowall Sniper");
	SetupValue(g_Options.aim_mindamage_sniper, "Aimbot", "Min Damage Sniper");
	SetupValue(g_Options.aim_type_sniper, "Aimbot", "Aim Type Sniper");
	SetupValue(g_Options.aim_hitbox_sniper, "Aimbot", "Aim Hitbox Sniper");

	SetupValue(g_Options.aim_fov_scout, "Aimbot", "FOV Scout");
	SetupValue(g_Options.aim_smooth_scout, "Aimbot", "Smooth Scout");
	SetupValue(g_Options.aim_silent_scout, "Aimbot", "Silent Scout");
	SetupValue(g_Options.aim_autowall_scout, "Aimbot", "Autowall Scout");
	SetupValue(g_Options.aim_mindamage_scout, "Aimbot", "Min Damage Scout");
	SetupValue(g_Options.aim_type_scout, "Aimbot", "Aim Type Scout");
	SetupValue(g_Options.aim_hitbox_scout, "Aimbot", "Aim Hitbox Scout");

	SetupValue(g_Options.aim_fov_smg, "Aimbot", "FOV SMG");
	SetupValue(g_Options.aim_smooth_smg, "Aimbot", "Smooth SMG");
	SetupValue(g_Options.aim_silent_smg, "Aimbot", "Silent SMG");
	SetupValue(g_Options.aim_rcs_smg, "Aimbot", "RCS SMG");
	SetupValue(g_Options.aim_rcs_start_smg, "Aimbot", "RCS Start SMG");
	SetupValue(g_Options.aim_rcs_type_smg, "Aimbot", "RCS Type SMG");
	SetupValue(g_Options.aim_rcs_x_smg, "Aimbot", "RCS X SMG");
	SetupValue(g_Options.aim_rcs_y_smg, "Aimbot", "RCS Y SMG");
	SetupValue(g_Options.aim_autowall_smg, "Aimbot", "Autowall SMG");
	SetupValue(g_Options.aim_mindamage_smg, "Aimbot", "Min Damage SMG");
	SetupValue(g_Options.aim_type_smg, "Aimbot", "Aim Type SMG");
	SetupValue(g_Options.aim_hitbox_smg, "Aimbot", "Aim Hitbox SMG");

	SetupValue(g_Options.aim_fov_heavy, "Aimbot", "FOV Heavy");
	SetupValue(g_Options.aim_smooth_heavy, "Aimbot", "Smooth Heavy");
	SetupValue(g_Options.aim_silent_heavy, "Aimbot", "Silent Heavy");
	SetupValue(g_Options.aim_rcs_heavy, "Aimbot", "RCS Heavy");
	SetupValue(g_Options.aim_rcs_start_heavy, "Aimbot", "RCS Start Heavy");
	SetupValue(g_Options.aim_rcs_type_heavy, "Aimbot", "RCS Type Heavy");
	SetupValue(g_Options.aim_rcs_x_heavy, "Aimbot", "RCS X Heavy");
	SetupValue(g_Options.aim_rcs_y_heavy, "Aimbot", "RCS Y Heavy");
	SetupValue(g_Options.aim_autowall_heavy, "Aimbot", "Autowall Heavy");
	SetupValue(g_Options.aim_mindamage_heavy, "Aimbot", "Min Damage Heavy");
	SetupValue(g_Options.aim_type_heavy, "Aimbot", "Aim Type Heavy");
	SetupValue(g_Options.aim_hitbox_heavy, "Aimbot", "Aim Hitbox Heavy");

	/*TRIGGERBOT*/
	SetupValue(g_Options.trigger_enabled, "Triggerbot", "Enabled");
	SetupValue(g_Options.trigger_key, "Triggerbot", "Key");
	SetupValue(g_Options.trigger_delay, "Triggerbot", "Delay");
	SetupValue(g_Options.trigger_head, "Triggerbot", "Head");
	SetupValue(g_Options.trigger_chest, "Triggerbot", "Chest");
	SetupValue(g_Options.trigger_stomach, "Triggerbot", "Stomach");
	SetupValue(g_Options.trigger_arms, "Triggerbot", "Arms");
	SetupValue(g_Options.trigger_legs, "Triggerbot", "Legs");

	/*ESP*/
	SetupValue(g_Options.esp_enabled, "ESP", "Enabled");
	SetupValue(g_Options.esp_visonly, "ESP", "Visible Only");
	SetupValue(g_Options.esp_player_boxes, "ESP", "Boxes");
	SetupValue(g_Options.esp_player_names, "ESP", "Names");
	SetupValue(g_Options.esp_player_health, "ESP", "Health");
	SetupValue(g_Options.esp_player_weapons, "ESP", "Weapon");

	SetupColor(g_Options.color_esp_enemy, "ESP Enemy");

	/*CHAMS*/
	SetupValue(g_Options.chams_player_enabled, "Chams", "Enabled");
	SetupValue(g_Options.chams_player_ignorez, "Chams", "Occluded");
	SetupValue(g_Options.player_enemies_shine, "Chams", "Shine");
	SetupValue(g_Options.chams_player_flat, "Chams", "Chams Type");
	SetupValue(g_Options.chams_player_backtrack, "Chams", "Backtrack");
	SetupValue(g_Options.chams_player_backtrack_type, "Chams", "Backtrack Type");
	SetupValue(g_Options.chams_player_backtrack_allticks, "Chams", "Show All Ticks");

	SetupColor(g_Options.color_chams_player_enemy_visible, "Chams Enemy Visible");
	SetupColor(g_Options.color_chams_player_enemy_occluded, "Chams Enemy Occluded");
	SetupColor(g_Options.color_chams_backtrack, "Chams Enemy Backtrack");
	SetupColor(g_Options.player_enemy_visible_shine, "Chams Shine");

	/*GLOW*/
	SetupValue(g_Options.glow_enabled, "Glow", "Enabled");
	SetupValue(g_Options.glow_enemiesOC, "Glow", "Occluded");
	SetupValue(g_Options.glow_enemies_type, "Glow", "Glow Type");
	SetupValue(g_Options.glow_weapons, "Glow", "Weapons");
	SetupValue(g_Options.glow_planted_c4, "Glow", "C4");

	SetupColor(g_Options.color_glow_enemy, "Glow Enemy Visible");
	SetupColor(g_Options.color_glow_enemyOC, "Glow Enemy Occluded");
	SetupColor(g_Options.color_glow_planted_c4, "Glow C4");
	SetupColor(g_Options.color_glow_weapons, "Glow Weapons");

	/*MISC*/
	SetupValue(g_Options.GrenadeIndicator, "Misc", "Grenade Prediction");
	SetupValue(g_Options.hitmarker, "Misc", "Hitmarker");
	SetupValue(g_Options.hitsound, "Misc", "Hitsound");
	SetupValue(g_Options.damage_indicator, "Misc", "Damage Indicator");
	SetupValue(g_Options.GrenadeIndicator, "Misc", "Night Mode");

	SetupColor(g_Options.color_grenade_indicator, "Grenade Prediction Line");
	SetupColor(g_Options.color_hitmarker, "Hitmarker Line");
	SetupColor(g_Options.color_damageindicator, "Damage Indicator Text");
}

void Options::SetupMisc()
{
	SetupColor(g_Options.Velocitycol, "Velocity");
	SetupValue(g_Options.misc_showranks, "Misc", "Rank reveal");
	SetupValue(g_Options.spectator_list, "Misc", "Spectator list");
	SetupValue(g_Options.misc_watermark, "Misc", "Watermark");
	SetupValue(g_Options.Velocity, "Misc", "Velocity");
	SetupValue(g_Options.LastJump , "Misc", "Last jump");
	SetupValue(g_Options.Graph, "Misc", "Graph");
	SetupValue(g_Options.autoaccept, "Misc", "Auto accept");
	SetupValue(g_Options.no_flash, "Misc", "No flash");
	SetupValue(g_Options.no_smoke, "Misc", "No smoke");
	SetupValue(g_Options.misc_bhop, "Misc", "Bunny hop");
	SetupValue(g_Options.autostrafe, "Misc", "Auto strafe");
	SetupValue(g_Options.AutoStafe_key, "Misc", "AutoStafe_key");
	SetupValue(g_Options.edge_bug, "Misc", "Edge bug");
	SetupValue(g_Options.edge_bug_key, "Misc", "edge_bug_key");
	SetupValue(g_Options.jump_bug, "Misc", "jump_bug");
	SetupValue(g_Options.jump_bug_key, "Misc", "jump_bug_key");
	SetupValue(g_Options.edgejump.enabled, "Misc", "Edge jump");
	SetupValue(g_Options.edgejump.hotkey, "Misc", "edgejump_key");
	SetupValue(g_Options.edgejump.edge_jump_duck_in_air, "Misc", "Duck in Air");
	SetupValue(g_Options.sniper_xhair, "Misc", "Sniper crosshair");
	SetupValue(g_Options.misc_bt, "Misc", "Backtrack");
	SetupValue(g_Options.bt_ticks, "Misc", "Backtrack ticks");
	SetupValue(g_Options.extended_bt, "Misc", "Extended backtrack");
	SetupValue(g_Options.misc_clantag, "Misc", "Clantag");
	SetupValue(g_Options.misc_clantag_type, "Misc", "Clantag Type");
	SetupValue(g_Options.misc_discordrpc, "Misc", "Discord RPC");
	//SetupValue(g_Options.misc_clantag_string[64], "Misc", "Clantag string");
	SetupValue(g_Options.edgejump.enabled, "Misc", "Edge jump");
	SetupValue(g_Options.playerModelCT, "Misc", "playerModelCT");
	SetupValue(g_Options.playerModelT, "Misc", "playerModelT");

}

void Options::Initialize()
{
	CHAR my_documents[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);
	folder = my_documents + std::string("\\CYBERWARE\\");
	CreateDirectoryA(folder.c_str(), nullptr);
	SetupVisuals();
	SetupMisc();
	SetupWeapons();
}

void Options::SaveSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	for (auto value : ints)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());

	for (auto value : floats)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
}

void Options::LoadSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}

	for (auto value : floats)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = float(atof(value_l));
	}
	g_ClientState->ForceFullUpdate();
}

void Options::DeleteSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	remove(file.c_str());
}