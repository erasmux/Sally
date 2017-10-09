#pragma once

#include <sally/common.hpp>
#include <sally/util/threading.hpp>
#include <vector>
#include <unordered_map>
#include <SDL_atomic.h>
#include <SDL_pixels.h>
#include <SDL_ttf.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;

namespace sally {

	struct Color
	{
		unsigned char _r;
		unsigned char _g;
		unsigned char _b;
		unsigned char _a;

		Color(unsigned char r_ = 0, unsigned char g_ = 0, unsigned char b_ = 0, unsigned char a_ = 255)
			: _r(r_), _g(g_), _b(b_), _a(a_) {}

		SDL_Color& sdl_color() { return *reinterpret_cast<SDL_Color*>(this); }
		const SDL_Color& sdl_color() const { return *reinterpret_cast<const SDL_Color*>(this); }
	};
	
	struct Rect {
		int _x, _y, _width, _height;

		Rect(int x_, int y_, int width_, int height_) : _x(x_), _y(y_), _width(width_), _height(height_) {}
		Rect() : Rect(0, 0, 0, 0) {}

		int center_x() const { return _x + _width / 2; }
		int center_y() const { return _y + _height / 2; }
		void set_center(int cx_, int cy_) {
			_x = cx_ - _width / 2;
			_y = cy_ - _height / 2;
		}
		void align_center(const Rect& other) { set_center(other.center_x(), other.center_y()); }

		SDL_Rect& sdl_rect() { return *reinterpret_cast<SDL_Rect*>(this); }
		const SDL_Rect& sdl_rect() const { return *reinterpret_cast<const SDL_Rect*>(this); }
	};

	class Font
	{
	public:
		enum render_mode_t {
			RENDER_SOLID,   // fastest but lowest quality (use for fast changing text)
			RENDER_BLENDED  // antialiased with alpha transperancy (use for high quality unboxed text)
		};

		TTF_Font* sdl_font() const { return _font; }

		void set_style(int style_);
		int get_style() const;

		int height() const;
		int lineskip() const;

		// calc_width return -1 on error
		int calc_width(const char* utf8_) const;
		int calc_width(const wchar_t* unicode_) const;

		virtual ~Font();

	private: // interface for FontManager
		Font(const std::string& filepath_, int ptsize_, long index_);
		friend class FontManager;

	private:
		TTF_Font* _font;
	};

	class FontManager
	{
	public:
		Font* load_font(const std::string& name_, const std::string& filepath_, int ptsize_, long index_ = 0) {
			Font* f = new Font(filepath_, ptsize_, index_); _map[name_].reset(f); return f;
		}

		Font* lookup(const std::string& name_) {
			auto find_it = _map.find(name_);
			return find_it != _map.end() ? find_it->second.get() : nullptr;
		}

		// be careful when unloading fonts, i.e. TextLine references the font object
		void erase(const std::string& name_) {
			_map.erase(name_);
		}

		void clear() { _map.clear(); }

	private:
		std::unordered_map<std::string, unique_ptr<Font> > _map;
	};

	class Renderer;

	class Renderable {
	public:
		// texture_for_render and fill_bounding_rect should only be called from main thread
		virtual SDL_Texture* texture_for_render(Renderer& renderer_) = 0;
		virtual void fill_bounding_rect(Renderer& renderer_, Rect& rect_) = 0;
		virtual ~Renderable() = default;
	};

	class Texture : public Renderable {
	public:
		int width() const { return _width; }
		int height() const { return _height; }

		virtual SDL_Texture* texture_for_render(Renderer& renderer_);
		virtual void fill_bounding_rect(Renderer& renderer_, Rect& rect_);

		Texture(SDL_Texture* texture_) : Texture() { reinit(texture_); }
		virtual ~Texture() { reinit(nullptr); }

	protected:
		Texture() : _width(0), _height(0), _texture(nullptr) {}
		void reinit(SDL_Texture* _texture);

		int _width, _height;
		SDL_Texture* _texture;
	};

	class TextLine : public Texture
	{
	public:
		TextLine(Font* font_, const Color& color_, const std::string& utf8_, Font::render_mode_t mode_ = Font::RENDER_BLENDED)
			: _font(font_), _mode(mode_), _text(utf8_), _color(color_), _cached({0})
		{}

		// when seting text, width and height will update only on next render
		void set_text(const std::string& utf8_) {
			spinlock::Guard lg(_lock);
			if (_text != utf8_) { _text = utf8_; SDL_AtomicSet(&_cached, 0); }
		}
		std::string get_text() const { spinlock::Guard lg(_lock);  return _text; }

		void set_color(const Color& color_) {
			spinlock::Guard lg(_lock);
			if (memcmp(&_color,&color_,sizeof(Color))!=0) { _color = color_; SDL_AtomicSet(&_cached, 0); }
		}
		Color get_color() const { spinlock::Guard lg(_lock);  return _color; }

		// cache and release texture should only be called from main thread
		void cache_texture(Renderer& render_);
		void release_texture() { reinit(0); SDL_AtomicSet(&_cached, 0); }

		virtual SDL_Texture* texture_for_render(Renderer& renderer_);
		virtual void fill_bounding_rect(Renderer& renderer_, Rect& rect_);

	private:
		Font* const _font;
		const Font::render_mode_t _mode;
		std::string _text;
		Color _color;
		SDL_atomic_t _cached;
		mutable spinlock _lock;
	};

	class Renderer { // only exists within a Window context
	public:
		Texture* load_image(const std::string& name_, const std::string& filepath_) {
			return insert(name_, new Texture(image_from_file(filepath_)));
		}

		Texture* render_text(const std::string& name_, Font* font_, const Color& color_, const std::string& utf8_, Font::render_mode_t mode_ = Font::RENDER_BLENDED) {
			return insert(name_, new Texture(render_sdl_text(font_, color_, utf8_, mode_)));
		}

		// renderable_ will be deleted by this object; replaces existing values
		template<typename R>
		R* insert(const std::string& name_, R* renderable_) {
			spinlock::Guard lg(_lock);
			_map[name_].reset(renderable_);
			return renderable_;
		}

		void erase(const std::string& name_) {
			spinlock::Guard lg(_lock);
			_map.erase(name_);
		}

		Renderable* lookup(const std::string& name_) {
			spinlock::Guard lg(_lock);
			auto find_it = _map.find(name_);
			return find_it != _map.end() ? find_it->second.get() : nullptr;
		}

		void render(Renderable* renderable_, const Rect& dst_, Rect* clip_ = nullptr)
			{ render_impl(renderable_, dst_, clip_, false); }
		void render(const std::string& name_, const Rect& dst_, Rect* clip_ = nullptr)
			{ render_impl(render_lookup(name_), dst_, clip_, false); }
		void render(Renderable* renderable_, int x_, int y_, Rect* clip_ = nullptr)
			{ render_impl(renderable_, Rect(x_,y_,0,0), clip_, true); }
		void render(const std::string& name_, int x_, int y_, Rect* clip_ = nullptr)
			{ render_impl(render_lookup(name_), Rect(x_,y_,0,0), clip_, true); }

		Color draw_color();
		void draw_color(const Color& color_);
		void fill_rect(const Rect& rect_);
		void draw_line(int x1_, int y1_, int x2_, int y2_);

		void clear();
		void clear(const Color& color_) { Color prev = draw_color(); draw_color(color_); clear(); draw_color(prev); }

		void begin_render() {}
		void end_render();

		// usefull to get render width and height (at least currently, x and y will always be 0).
		Rect output_rect() const;

	public: // interface for text renderables
		SDL_Texture* render_sdl_text(Font* font_, const Color& color_, const std::string& utf8_, Font::render_mode_t mode_);

	private: // interface for Window
		Renderer() : _renderer(nullptr) {}
		void initialize(SDL_Window* window_);
		virtual ~Renderer();
		SDL_Renderer* _renderer;
		friend class Window;

	private:
		Renderable* render_lookup(const std::string& name_);
		void render_impl(Renderable* renderable_, const Rect& dst_, Rect* clip_, bool override_dst_wh_);
		SDL_Texture* image_from_file(const std::string& filepath_);

		std::unordered_map<std::string, unique_ptr<Renderable> > _map;
		mutable spinlock _lock;
	};

	class RenderProvider {
	public:
		virtual void initialize(Window& win_) {}
		virtual void render(Window& win_) = 0;
		virtual ~RenderProvider() = default;

		typedef generic_guard<const RenderProvider> Guard;
		void lock() const { _mutex.lock(); }
		void unlock() const { _mutex.unlock();  }
		bool try_lock() const { return _mutex.try_lock(); }
	private:
		mutable mutex _mutex;
	};

	// windows + rendering tasks should only be done from main thread.
	// to aid this, the window ctor will register with the System::window_manager()
	// (hence the Window objects should be created in the main thread), and the
	// System::main_loop() will do the actual rendering. Thread can call and should
	// call invalide() when the should be redrawn, this will cause the RenderProvider
	// to be locked and it's render() function called from the main thread.
	class Window {
	public:
		typedef unsigned int id_t;

		typedef unsigned int flags_t; // bit mask of the following flags:
		enum {
			FLG_DEFAULT = 0,
			FLG_FULLSCREEN = 1, // obeys width and height
			FLG_FULLSCREEN_DESKTOP = 2, // use desktop width and height
			FLG_BORDERLESS = 4
		};

		// use width & height = -1 for full screen
		Window(const char* title_, int width_, int height_, flags_t flags_, RenderProvider* rprovider_);
		~Window();

		Renderer& renderer() { return _renderer; }

		int width() const { return _width; }
		int height() const { return _height; }
		id_t id() const { return _id; }

		void invalidate();
		void validate();

		bool render_pending() const;
		void render();

	private:
		int _width, _height;
		id_t _id;

		Uint32 sdl_flags(flags_t flags_);

		SDL_Window* _window;
		Renderer _renderer;
		RenderProvider* const _rprovider; // const to avoid multi-threading issues
		SDL_atomic_t _render_pending;
	};

	class WindowManager
	{
	public:
		void register_win(Window* win_);
		void unregister_win(Window* win_);

		void render_all_pending();
		void invalidate_all();

		Window* window_by_id(Window::id_t id_) {
			auto find_it = _windows.find(id_);
			return find_it != _windows.end() ? find_it->second : nullptr;
		}

	private:
		std::unordered_map<Window::id_t, Window*> _windows;
	};

}
