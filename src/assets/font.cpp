
#include <sally/assets/font.hpp>
#include <SDL_ttf.h>

namespace sally {

	font::font(const std::string& filepath_, int ptsize_, long index_)
	{
		_font = TTF_OpenFontIndex(filepath_.c_str(), ptsize_, index_);
		if (!_font)
			throw ttf_exception("TTF_OpenFontIndex failed", filepath_.c_str());
	}

	font::~font()
	{
		TTF_CloseFont(_font);
	}

	void font::set_style(int style_)
	{
		TTF_SetFontStyle(_font, style_);
	}

	int font::get_style() const
	{
		return TTF_GetFontStyle(_font);
	}

	int font::height() const
	{
		return TTF_FontHeight(_font);
	}

	int font::lineskip() const
	{
		return TTF_FontLineSkip(_font);
	}

	int font::calc_width(const char* utf8_) const
	{
		int w = -1;
		return TTF_SizeUTF8(_font, utf8_, &w, nullptr) == 0 ? w : -1;
	}

	int font::calc_width(const wchar_t* unicode_) const
	{
		if (sizeof(wchar_t) != sizeof(Uint16))
			throw general_exception("calc_width on unicode string not supported when sizeof(wchar_t) != sizeof(Uint16)");
		int w = -1;
		return TTF_SizeUNICODE(_font, reinterpret_cast<const Uint16*>(unicode_), &w, nullptr) == 0 ? w : -1;
	}

}
