#include <sally/gfx/basics.hpp>

#include <SDL.h>

namespace sally {

	static_assert(sizeof(color) == sizeof(SDL_Color), "sally::color must be memory compatible with SDL_Color");
	static_assert(sizeof(rectangle) == sizeof(SDL_Rect), "sally::rectangle must be memory compatible with SDL_Rect");

}
