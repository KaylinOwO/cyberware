#include "glow.hpp"

#include "../../valve_sdk/csgostructs.hpp"
#include "../../options.hpp"

Glow::Glow()
{
}

Glow::~Glow()
{
    // We cannot call shutdown here unfortunately.
    // Reason is not very straightforward but anyways:
    // - This destructor will be called when the dll unloads
    //   but it cannot distinguish between manual unload 
    //   (pressing the Unload button or calling FreeLibrary)
    //   or unload due to game exit.
    //   What that means is that this destructor will be called
    //   when the game exits.
    // - When the game is exiting, other dlls might already 
    //   have been unloaded before us, so it is not safe to 
    //   access intermodular variables or functions.
    //   
    //   Trying to call Shutdown here will crash CSGO when it is
    //   exiting (because we try to access g_GlowObjManager).
    //
}

void Glow::Shutdown()
{
    // Remove glow from all entities
    for(auto i = 0; i < g_GlowObjManager->glowObjectDefinitions.Count(); i++) {
        auto& glowObject = g_GlowObjManager->glowObjectDefinitions[i];
        auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.entity);

        if(glowObject.isUnused())
            continue;

        if(!entity || entity->IsDormant())
            continue;

        glowObject.glowAlpha = 0.0f;
    }
}

void Glow::Run()
{
  
    for(auto i = 0; i < g_GlowObjManager->glowObjectDefinitions.Count(); i++) {
        auto& glowObject = g_GlowObjManager->glowObjectDefinitions[i];
        auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.entity);

        if(glowObject.isUnused())
            continue;

        if(!entity || entity->IsDormant())
            continue;

        if (g_Options.glow_enabled && g_Options.glow_players && entity->IsPlayer() && entity->IsEnemy())
        {
            if (!g_Options.glow_enemiesOC && !(entity->VisibleTo(g_LocalPlayer))) continue;
            if (g_Options.glow_enemiesOC && !(entity->VisibleTo(g_LocalPlayer)))
            {
                glowObject.glowColor[0] = g_Options.color_glow_enemyOC[0] / 255.0f;
                glowObject.glowColor[1] = g_Options.color_glow_enemyOC[1] / 255.0f;
                glowObject.glowColor[2] = g_Options.color_glow_enemyOC[2] / 255.0f;
                glowObject.glowAlpha = g_Options.color_glow_enemyOC[3] / 255.0f;
                glowObject.glowStyle = 0;
            }
            else
            {
                glowObject.glowColor[0] = g_Options.color_glow_enemy[0] / 255.0f;
                glowObject.glowColor[1] = g_Options.color_glow_enemy[1] / 255.0f;
                glowObject.glowColor[2] = g_Options.color_glow_enemy[2] / 255.0f;
                glowObject.glowAlpha = g_Options.color_glow_enemy[3] / 255.0f;
                glowObject.glowStyle = g_Options.glow_enemies_type;
            }
        }

        if (g_Options.glow_weapons && entity->IsWeapon())
        {
            glowObject.glowColor[0] = g_Options.color_glow_weapons[0] / 255.0f;
            glowObject.glowColor[1] = g_Options.color_glow_weapons[1] / 255.0f;
            glowObject.glowColor[2] = g_Options.color_glow_weapons[2] / 255.0f;
            glowObject.glowAlpha = g_Options.color_glow_weapons[3] / 255.0f;
            glowObject.glowStyle = 0;
        }

        if (g_Options.glow_planted_c4 && entity->IsPlantedC4())
        {
            glowObject.glowColor[0] = g_Options.color_glow_planted_c4[0] / 255.0f;
            glowObject.glowColor[1] = g_Options.color_glow_planted_c4[1] / 255.0f;
            glowObject.glowColor[2] = g_Options.color_glow_planted_c4[2] / 255.0f;
            glowObject.glowAlpha = g_Options.color_glow_planted_c4[3] / 255.0f;
            glowObject.glowStyle = 0;
        }


        glowObject.renderWhenOccluded = true;
        glowObject.renderWhenUnoccluded = false;

    }
}
