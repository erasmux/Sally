
#include <sally/assets/texture.hpp>
#include <SDL.h>

namespace sally {

	texture::texture(SDL_Texture* texture_)
		: _texture(texture_)
	{
		Uint32 format = 0;
		int access = 0;
		if (SDL_QueryTexture(_texture, &format, &access, &_size._width, &_size._height))
			throw sdl_exception("SDL_QueryTexture failed", SDL_GetError());
	}

	texture::texture(SDL_Renderer* renderer_, SDL_Surface* surface_)
		: texture(safe_texture_from_surface(renderer_,surface_))
	{
	}

	texture::~texture()
	{
		SDL_DestroyTexture(_texture);
	}

	SDL_Texture* texture::safe_texture_from_surface(SDL_Renderer* renderer_, SDL_Surface* surface_)
	{
		if (SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface_))
			return texture;
		else
			throw sdl_exception("SDL_CreateTextureFromSurface failed", SDL_GetError());
	}

}
