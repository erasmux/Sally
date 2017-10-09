#pragma once

#include <sally/common.hpp>

struct _TTF_Font;

namespace sally {

	class font
	{
	public:
		enum render_mode_t {
			RENDER_SOLID,   // fastest but lowest quality (use for fast changing text)
			RENDER_BLENDED  // antialiased with alpha transperancy (use for high quality unboxed text)
		};

		void set_style(int style_);
		int get_style() const;

		int height() const;
		int lineskip() const;

		// calc_width return -1 on error
		int calc_width(const char* utf8_) const;
		int calc_width(const wchar_t* unicode_) const;

		~font();

	public: // interface for sally library
		// to load fonts use the asset manager
		font(const std::string& filepath_, int ptsize_, long index_);

		_TTF_Font* sdl_font() const { return _font; }

	private:
		_TTF_Font* _font;
	};

}
