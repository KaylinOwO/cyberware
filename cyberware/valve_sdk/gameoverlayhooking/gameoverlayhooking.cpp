#include "gameoverlayhooking.h"
#include "pe/pe.h"
#include "pe/util/util.h"
#include "../../options.hpp"
//  Prototypes
GOHooking::Functions::Types::game_overlay_renderer_hook_t GOHooking::Functions::Prototypes::o_hook;
GOHooking::Functions::Types::game_overlay_renderer_unhook_t GOHooking::Functions::Prototypes::o_unhook;

namespace GOHooking {
	namespace Functions {
		//  declare initialization
		namespace Initialize {
			bool the() {
				GOHooking::Functions::Prototypes::o_hook = (GOHooking::Functions::Types::game_overlay_renderer_hook_t)pe::util::prototypes::static_addresses[pe::util::StaticAddresses::ADDRESS_GAME_OVERAY_RENDERER_HOOKING];

				if (GOHooking::Functions::Prototypes::o_hook == nullptr) {
					return false;
				}

				GOHooking::Functions::Prototypes::o_unhook = (GOHooking::Functions::Types::game_overlay_renderer_unhook_t)pe::util::prototypes::static_addresses[pe::util::StaticAddresses::ADDRESS_GAME_OVERAY_RENDERER_UNHOOKING];

				if (GOHooking::Functions::Prototypes::o_unhook == nullptr) {
					return false;
				}

				GOHooking::Functions::Initialize::have_values_been_initialized = true;

				if (!GOHooking::Functions::apply_hook<GOHooking::Hooks::GetColorModulation>(pe::util::prototypes::static_addresses[pe::util::StaticAddresses::ADDRESS_GET_COLOR_MODULATION])) {
					return false;
				}

				if (!GOHooking::Functions::apply_hook<GOHooking::Hooks::IsUsingStaticProp>(pe::util::prototypes::static_addresses[pe::util::StaticAddresses::ADDRESS_IS_USING_STATIC_PROP_DEBUG_MODES])) {
					return false;
				}

				return true;
			}
		}
	}


	void __fastcall GOHooking::Hooks::GetColorModulation::hooked(REGISTERS, float* r, float* g, float* b) {
		o_fn(thisptr, dummy, r, g, b);

		if (g_Options.world_modulation)
		{
			const auto material = reinterpret_cast<IMaterial*>(thisptr);

			auto name = material->GetName();
			auto group = material->GetTextureGroupName();

			// exclude stuff we dont want modulated
			if (strstr(group, "Other") || strstr(name, "player") || strstr(name, "chams") ||
				strstr(name, "weapon") || strstr(name, "glow"))
				return;

			bool is_prop = strstr(group, "StaticProp");

			*r *= is_prop ? 0.5f /* * 2.5*/: 0.2f;
			*g *= is_prop ? 0.5f : 0.2f;
			*b *= is_prop ? 0.5f : 0.2f;
		//	const auto shader_parameter = material->GetShaderParams()[5];
			//shader_parameter->set_float(is_prop ? 0.8f : 1.0f);
		}
	}

	bool __cdecl GOHooking::Hooks::IsUsingStaticProp::hooked() {
		return g_Options.world_modulation;
	}
}


