#pragma once

#include <sally/gfx/basics.hpp>

struct SDL_Texture;
struct SDL_Renderer;
struct SDL_Surface;

namespace sally {

	class texture {
	public:
		size_2d size() const { return _size;  }

		~texture();

	public: // interface for sally library
		// to load textures use the asset manager
		texture(SDL_Texture* texture_); // texture_ must be valid
		texture(SDL_Renderer* renderer_, SDL_Surface* surface_);

		SDL_Texture* sdl_texture() const { return _texture; }

	private:
		SDL_Texture* safe_texture_from_surface(SDL_Renderer* renderer_, SDL_Surface* surface_);

		SDL_Texture* _texture;
		size_2d _size;
	};

}
