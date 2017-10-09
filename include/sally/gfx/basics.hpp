#pragma once

#include <sally/common.hpp>

struct SDL_Color;
struct SDL_Rect;

namespace sally {

	typedef int screen_unit;

	struct color
	{
		typedef unsigned char component_type;

		component_type _r;
		component_type _g;
		component_type _b;
		component_type _a;

		color(component_type r_, component_type g_, component_type b_, component_type a_ = 255)
			: _r(r_), _g(g_), _b(b_), _a(a_)
		{}
		color() : color(0, 0, 0) {}

		SDL_Color& sdl_color() { return *reinterpret_cast<SDL_Color*>(this); }
		const SDL_Color& sdl_color() const { return *reinterpret_cast<const SDL_Color*>(this); }
	};

	struct size_2d {
		screen_unit _width;
		screen_unit _height;

		size_2d(screen_unit width_, screen_unit height_)
			: _width(width_), _height(height_)
		{}

		size_2d() : size_2d(0, 0) {}
	};

	struct point {
		screen_unit _x;
		screen_unit _y;

		point(screen_unit x_, screen_unit y_)
			: _x(x_), _y(y_)
		{}
		point() : point(0, 0) {}
	};

	inline point operator+(const point& p_, const point& q_) {
		return point(p_._x + q_._x, p_._y + p_._y);
	}
	inline size_2d operator-(const point& p_, const point& q_) {
		return size_2d(p_._x - q_._x, p_._y - q_._y);
	}
	inline point operator+(const point& p_, const size_2d& s_) {
		return point(p_._x + s_._width, p_._y + s_._height);
	}

	struct rectangle {
		screen_unit _x;
		screen_unit _y;
		screen_unit _width;
		screen_unit _height;

		rectangle(screen_unit x_, screen_unit y_, screen_unit width_, screen_unit height_)
			: _x(x_), _y(y_), _width(width_), _height(height_)
		{}
		rectangle(const point& top_left_, const size_2d& size_)
			: _x(top_left_._x), _y(top_left_._y), _width(size_._width), _height(size_._height)
		{}
		rectangle(const point& top_left_, const point& bottom_right_)
			: rectangle(top_left_, bottom_right_ - top_left_)
		{}
		rectangle() : rectangle(0, 0, 0, 0) {}

		const point top_left() { return point(_x, _y); }
		const size_2d size() { return size_2d(_width, _height); }
		// notice rectangle does not actually include the bottom_right point
		const point bottom_right() { return point(_x + _width, _y + _height); }

		void set_top_left(const point& top_left_) { _x = top_left_._x; _y = top_left_._y; }
		void set_bottom_right(const point& bottom_right_) { set_size(bottom_right_ - top_left()); }
		void set_size(const size_2d& size_) { _width = size_._width; _height = size_._height; }

		SDL_Rect& sdl_rect() { return *reinterpret_cast<SDL_Rect*>(this); }
		const SDL_Rect& sdl_rect() const { return *reinterpret_cast<const SDL_Rect*>(this); }
	};

}
