#include <sally/gfx.hpp>
#include <sally/util/logger.hpp>
#include <sally/system.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <algorithm>

namespace sally {

	// Font:

	Font::Font(const std::string& filepath_, int ptsize_, long index_)
	{
		_font = TTF_OpenFontIndex(filepath_.c_str(), ptsize_, index_);
		if (!_font)
			throw ttf_exception("TTF_OpenFontIndex failed", filepath_.c_str());
	}

	Font::~Font()
	{
		TTF_CloseFont(_font);
	}

	void Font::set_style(int style_)
	{
		TTF_SetFontStyle(_font, style_);
	}

	int Font::get_style() const
	{
		return TTF_GetFontStyle(_font);
	}

	int Font::height() const
	{
		return TTF_FontHeight(_font);
	}

	int Font::lineskip() const
	{
		return TTF_FontLineSkip(_font);
	}

	int Font::calc_width(const char* utf8_) const
	{
		int w = -1;
		return TTF_SizeUTF8(_font, utf8_, &w, nullptr) == 0 ? w : -1;
	}

	int Font::calc_width(const wchar_t* unicode_) const
	{
		if (sizeof(wchar_t) != sizeof(Uint16))
			throw general_exception("calc_width on unicode string not supported when sizeof(wchar_t) != sizeof(Uint16)");
		int w = -1;
		return TTF_SizeUNICODE(_font, reinterpret_cast<const Uint16*>(unicode_), &w, nullptr) == 0 ? w : -1;
	}

	// Texture:

	void Texture::reinit(SDL_Texture* texture_)
	{
		if (_texture)
			SDL_DestroyTexture(_texture);
		_texture = texture_;
		if (_texture && SDL_QueryTexture(_texture, nullptr, nullptr, &_width, &_height) != 0)
			throw sdl_exception("new texture SDL_QueryTexture failed");
	}

	SDL_Texture* Texture::texture_for_render(Renderer& renderer_)
	{
		return _texture;
	}

	void Texture::fill_bounding_rect(Renderer& renderer_, Rect& rect_)
	{
		rect_._x = 0;
		rect_._y = 0;
		rect_._width = _width;
		rect_._height = _height;
	}

	// TextLine:

	void TextLine::cache_texture(Renderer& renderer_)
	{
		if (SDL_AtomicGet(&_cached))
			return;
		spinlock::Guard lg(_lock);
		std::string text = _text;
		Color color = _color;
		SDL_AtomicSet(&_cached, 1);
		lg.unlock();
		reinit(renderer_.render_sdl_text(_font, color, text, _mode));
	}

	SDL_Texture* TextLine::texture_for_render(Renderer& renderer_)
	{
		cache_texture(renderer_);
		return Texture::texture_for_render(renderer_);
	}

	void TextLine::fill_bounding_rect(Renderer& renderer_, Rect& rect_)
	{
		cache_texture(renderer_);
		return Texture::fill_bounding_rect(renderer_, rect_);
	}

	// Renderer:

	void Renderer::initialize(SDL_Window* window_)
	{
		_renderer = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!_renderer)
			throw sdl_exception("SDL_CreateRenderer failed");
	}
	
	Renderer::~Renderer()
	{
		if (_renderer) {
			SDL_DestroyRenderer(_renderer);
			_renderer = nullptr;
		}
	}

	SDL_Texture* Renderer::image_from_file(const std::string& filepath_)
	{
		SDL_Texture* texture = IMG_LoadTexture(_renderer, filepath_.c_str());
		if (!texture)
			throw img_exception("IMG_LoadTexture failed", filepath_.c_str());
		return texture;
	}

	SDL_Texture* Renderer::render_sdl_text(Font* font_, const Color& color_, const std::string& utf8_, Font::render_mode_t mode_)
	{
		//logi() << "DEBUG: rendering text: " << utf8_;

		SDL_Surface *surf = nullptr;
		switch (mode_)
		{
		case Font::RENDER_SOLID:
			surf = TTF_RenderText_Solid(font_->sdl_font(), utf8_.c_str(), color_.sdl_color());
			if (!surf)
				throw ttf_exception("TTF_RenderText_Solid failed", utf8_.c_str());
			break;
		case Font::RENDER_BLENDED:
			surf = TTF_RenderText_Blended(font_->sdl_font(), utf8_.c_str(), color_.sdl_color());
			if (!surf)
				throw ttf_exception("TTF_RenderText_Blended failed", utf8_.c_str());
			break;
		}
		if (!surf)
			throw general_exception("invalid text render mode?!");

		SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surf);
		SDL_FreeSurface(surf);
		if (!texture)
			throw sdl_exception("SDL_CreateTextureFromSurface failed (rendering text)");
		
		return texture;
	}

	Renderable* Renderer::render_lookup(const std::string& name_)
	{
		if (Renderable* res = lookup(name_))
			return res;
		else {
			std::ostringstream err;
			err << "object for render not found <" << name_ << ">";
			throw general_exception(err.str().c_str());
		}
	}

	void Renderer::render_impl(Renderable* renderable_, const Rect& dst_, Rect* clip_, bool override_dst_wh_)
	{
		SDL_Texture* texture = renderable_ ? renderable_->texture_for_render(*this) : nullptr;
		if (!texture) {
			std::ostringstream err;
			err << "trying to render null texture?! <" << renderable_ << ">";
			throw general_exception(err.str().c_str());
		}

		const SDL_Rect* dst_rect = 0;
		Rect odst;
		if (override_dst_wh_)
		{
			if (clip_) {
				odst._width = clip_->_width;
				odst._height = clip_->_height;
			}
			else
				renderable_->fill_bounding_rect(*this, odst);
			odst._x = dst_._x;
			odst._y = dst_._y;
			dst_rect = &odst.sdl_rect();
		}
		else
			dst_rect = &dst_.sdl_rect();

		SDL_RenderCopy(_renderer, texture, clip_ ? &clip_->sdl_rect() : nullptr, dst_rect);
	}

	Color Renderer::draw_color()
	{
		Color cc;
		SDL_GetRenderDrawColor(_renderer, &cc._r, &cc._g, &cc._b, &cc._a);
		return cc;
	}
	
	void Renderer::draw_color(const Color& color_)
	{
		SDL_SetRenderDrawColor(_renderer, color_._r, color_._g, color_._b, color_._a);
	}

	void Renderer::fill_rect(const Rect& rect_)
	{
		SDL_RenderFillRect(_renderer, &rect_.sdl_rect());
	}

	void Renderer::draw_line(int x1_, int y1_, int x2_, int y2_)
	{
		SDL_RenderDrawLine(_renderer, x1_, y1_, x2_, y2_);
	}

	void Renderer::clear()
	{
		SDL_RenderClear(_renderer);
	}

	void Renderer::end_render()
	{
		SDL_RenderPresent(_renderer);
	}

	Rect Renderer::output_rect() const
	{
		Rect res;
		SDL_GetRendererOutputSize(_renderer, &res._width, &res._height);
		return res;
	}

	// Window:

	Uint32 Window::sdl_flags(flags_t flags_) {
		Uint32 res = 0;
		if (flags_ & FLG_FULLSCREEN_DESKTOP)
			res |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		else if (flags_ & FLG_FULLSCREEN)
			res |= SDL_WINDOW_FULLSCREEN;
		if (flags_ & FLG_BORDERLESS)
			res |= SDL_WINDOW_BORDERLESS;
		return res;
	}

	Window::Window(const char* title_, int width_, int height_, flags_t flags_, RenderProvider* rprovider_)
		: _width(width_), _height(height_), _id(0), _window(nullptr), _rprovider(rprovider_), _render_pending({ 1 })
	{
		_window = SDL_CreateWindow(title_, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width_, height_, sdl_flags(flags_));
		if (!_window)
			throw sdl_exception("SDL_CreateWindow failed");

		try {
			_id = SDL_GetWindowID(_window);
			_renderer.initialize(_window);

			const Rect& osz = _renderer.output_rect();
			_width = osz._width;
			_height = osz._height;
			
			if (_rprovider)
				_rprovider->initialize(*this);
			System::window_manger().register_win(this);
		}
		catch (...) {
			SDL_DestroyWindow(_window);
			throw;
		}
	}

	void Window::invalidate()
	{
		SDL_AtomicSet(&_render_pending, 1);
		System::request_render();
	}

	void Window::validate()
	{
		SDL_AtomicSet(&_render_pending, 0);
	}
	
	bool Window::render_pending() const
	{
		return SDL_AtomicGet(const_cast<SDL_atomic_t*>(&_render_pending)) != 0;
	}

	void Window::render()
	{
		// logi() << "rending window " << _id << "...";

		_renderer.begin_render();
		if (_rprovider) {
			RenderProvider::Guard rg(*_rprovider);
			_rprovider->render(*this);
			validate(); // while holding provider lock!
		}
		else validate();
		_renderer.end_render();
	}

	Window::~Window()
	{
		if (_window) {
			SDL_DestroyWindow(_window);
			_window = nullptr;
		}
		System::window_manger().unregister_win(this);
	}

	// WindowManager:

	void WindowManager::register_win(Window* win_)
	{
		_windows[win_->id()] = win_;
	}

	void WindowManager::unregister_win(Window* win_)
	{
		_windows.erase(win_->id());
	}

	void WindowManager::render_all_pending()
	{
		for (auto it = _windows.begin(); it != _windows.end(); ++it)
			if (it->second->render_pending())
				it->second->render();
	}

	void WindowManager::invalidate_all()
	{
		for (auto it = _windows.begin(); it != _windows.end(); ++it)
			it->second->invalidate();
	}

}