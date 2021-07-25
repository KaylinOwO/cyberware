#pragma once

#include "UtlVector.hpp"
#include "../Interfaces/IClientEntity.hpp"

struct GlowObjectDefinition {
    C_BasePlayer* entity;

    // TODO: try to declare those as std::array<float, 4> for easier color copying
    Vector glowColor;
    float glowAlpha;
    //

    bool glowAlphaCappedByRenderAlpha;
    float glowAlphaFunctionOfMaxVelocity;
    float glowAlphaMax;
    float glowPulseOverdrive;
    bool renderWhenOccluded;
    bool renderWhenUnoccluded;
    bool fullBloomRender;
    int fullBloomStencilTestValue;
    int glowStyle;
    int splitScreenSlot;
    int nextFreeSlot;

    constexpr bool isUnused() const noexcept { return nextFreeSlot != ENTRY_IN_USE; }

    static constexpr int END_OF_FREE_LIST = -1;
    static constexpr int ENTRY_IN_USE = -2;
};

struct GlowObjectManager {
    CUtlVector<GlowObjectDefinition> glowObjectDefinitions;
    constexpr bool hasGlowEffect(C_BasePlayer* entity) noexcept
    {
        for (int i = 0; i < glowObjectDefinitions.m_Size; i++)
            if (!glowObjectDefinitions[i].isUnused() && glowObjectDefinitions[i].entity == entity)
                return true;

        return false;
    }

    int firstFreeSlot;
};