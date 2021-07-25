#include "pe.h"

#include "util/util.h"

//  Prototypes
pe::types::image_type pe::prototypes::images[pe::Images::IMAGES_LIST_SIZE];

namespace pe {
	//  declare initializer
	namespace init {
		bool the() {
			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_CLIENT>("client.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_ENGINE>("engine.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_INPUT_SYSTEM>("inputsystem.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_VSTDLIB>("vstdlib.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_VGUI2>("vgui2.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_VGUI_MAT_SURFACE>("vguimatsurface.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_MATERIAL_SYSTEM>("materialsystem.dll")) {
				return false;
			}

			if (!pe::getter::the<pe::prototypes::images, pe::Images::IMAGE_GAME_OVERLAY_RENDERER>("GameOverlayRenderer.dll")) {
				return false;
			}

			if (!pe::util::init::the()) {
				return false;
			}

			return true;
		}
	}

}