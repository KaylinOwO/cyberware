#pragma once

#define WIN32_LEAN_AND_MEAN             //  Exclude rarely-used stuff from Windows headers
//  Windows Header Files
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <cstdint>

#include <assert.h>

#define REGISTERS void* thisptr, void* dummy

#define REGISTERS_t(x) x thisptr, void* dummy

namespace pe {
	enum Images {
		IMAGE_CLIENT,
		IMAGE_ENGINE,
		IMAGE_INPUT_SYSTEM,
		IMAGE_VSTDLIB,
		IMAGE_VGUI2,
		IMAGE_VGUI_MAT_SURFACE,
		IMAGE_MATERIAL_SYSTEM,
		IMAGE_GAME_OVERLAY_RENDERER,

		IMAGES_LIST_SIZE
	};

	namespace init {
		bool the();
	}

	namespace types {
		typedef HMODULE image_type;
	}

	namespace prototypes {
		extern pe::types::image_type images[Images::IMAGES_LIST_SIZE];
	}

	namespace getter {
		template <pe::types::image_type(&image_map)[Images::IMAGES_LIST_SIZE], pe::Images index, size_t N>
		inline bool the(const char(&image)[N]) {
			//  Assert that image map is valid
			assert(image_map);

			//  Get result now for valid checks
			const pe::types::image_type result = GetModuleHandleA(image);
			if (result == NULL) {
				return false;
			}

			//  Assign value to image map
			image_map[index] = result;

			return true;
		}
	}
}