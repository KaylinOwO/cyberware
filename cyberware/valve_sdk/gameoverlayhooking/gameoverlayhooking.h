#pragma once

#include "../sdk.hpp"

#define PROTOTYPE_FUNCTION(function_name, function_prototype) \
struct function_name \
{ \
	using fn = function_prototype; \
	static fn hooked; \
	static fn* o_fn; \
}; \
inline function_name::fn* function_name::o_fn;

namespace GOHooking {
	enum StaticAddresses {
		ADDRESS_GET_COLOR_MODULATION,
		ADDRESS_IS_USING_STATIC_PROP_DEBUG_MODES,

		STATIC_ADDRESSES_LIST_SIZE
	};
	namespace Patterns {

	}

	namespace Functions {
		namespace Initialize {
			static bool have_values_been_initialized = false;
			bool the();
		}

		namespace Detach {
			void the();
		}

		namespace Types {
			typedef bool(__cdecl* game_overlay_renderer_hook_t)(void*, void*, void*, int);
			typedef void(__cdecl* game_overlay_renderer_unhook_t)(void*, bool);
		}

		namespace Prototypes {
			extern Types::game_overlay_renderer_hook_t o_hook;
			extern Types::game_overlay_renderer_unhook_t o_unhook;
		}

		template <typename T>
		inline bool apply_hook(void* address) {
			assert(Initialize::have_values_been_initialized);
			return Prototypes::o_hook(address, T::hooked, &T::o_fn, 0);
		}

		inline void remove_hook(void* address) {
			assert(Initialize::have_values_been_initialized);
			Prototypes::o_unhook(address, false);
		}
	}
	namespace Hooks {
		PROTOTYPE_FUNCTION
		(
			GetColorModulation,
			void(__fastcall)(void*, void*, float*, float*, float*)
		);

		PROTOTYPE_FUNCTION
		(
			IsUsingStaticProp,
			bool(__cdecl)()
		);
	}
}