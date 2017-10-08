#include <sally/common.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

namespace sally {

	ticks_t clock_tick() { return SDL_GetTicks(); }

	std::string format_error_message(const char* msg_, const char* arg1_, const char* arg2_, const char* arg3_, const char* err_);

	const char* exception::what() const SALLY_NOEXCEPT
	{
		return "unknown Sally exception";
	}

	const char* general_exception::what() const SALLY_NOEXCEPT
	{
		return _what.c_str();
	}

	sdl_exception::sdl_exception(const char* msg_, const char* arg1_, const char* arg2_ , const char* arg3_)
		: _what(format_error_message(msg_, arg1_, arg2_, arg3_, SDL_GetError()))
	{
	}

	const char* sdl_exception::what() const SALLY_NOEXCEPT
	{
		return _what.c_str();
	}

	img_exception::img_exception(const char* msg_, const char* arg1_, const char* arg2_ , const char* arg3_)
		: _what(format_error_message(msg_, arg1_, arg2_, arg3_, IMG_GetError()))
	{
	}

	const char* img_exception::what() const SALLY_NOEXCEPT
	{
		return _what.c_str();
	}

	ttf_exception::ttf_exception(const char* msg_, const char* arg1_, const char* arg2_ , const char* arg3_)
		: _what(format_error_message(msg_, arg1_, arg2_, arg3_, TTF_GetError()))
	{
	}

	const char* ttf_exception::what() const SALLY_NOEXCEPT
	{
		return _what.c_str();
	}

	std::string format_error_message(const char* msg_, const char* arg1_, const char* arg2_, const char* arg3_, const char* err_)
	{
		std::string res;
		res.reserve(strlen_s(msg_) + strlen_s(err_) + strlen_s(arg1_) + strlen_s(arg2_) + strlen_s(arg3_) + 8);
		res += msg_;
		if (arg1_ || arg2_ || arg3_) {
			res += " <";
			if (arg1_)
				res += arg1_;
			if (arg2_ || arg3_) {
				res += ",";
				if (arg2_)
					res += arg2_;
				if (arg3_) {
					res += ",";
					res += arg3_;
				}
			}
			res += ">";
		}
		res += " : ";
		res += err_;
		return res;
	}

}
