#pragma once

#include <sally/sally.hpp>

class sliding_pawn : public sally::keyboard_event_handler, public sally::RenderProvider
{
public:
	enum { BOARD_WIDTH = 8, BOARD_HEIGHT = 8 };
	enum { START_POS_X = 3, START_POS_Y = 6 };
	enum { TILE_WIDTH = 64, TILE_HEIGHT = 64 };
	enum { WINDOW_WIDTH = BOARD_WIDTH*TILE_WIDTH, WINDOW_HEIGHT = BOARD_HEIGHT*TILE_HEIGHT };

	sliding_pawn() : _px(3), _py(6) {}

	virtual void initialize(sally::Window& win_) {
		using namespace sally;
		win_.renderer().load_image("pawn", System::resouce_path("examples/SlidingPawn/chess_pawn.png"));
	}

	virtual void render(sally::Window& win_) {
		using namespace sally;

		static const sally::Color white{ 255, 255, 255 };
		static const sally::Color black{   0,   0,   0 };

		logi() << ">> rending scene ...";

		Renderer& rend = win_.renderer();
		rend.clear();

		// draw board:
		for (int ii = 0; ii < BOARD_WIDTH; ++ii)
			for (int jj = 0; jj < BOARD_HEIGHT; ++jj)
			{
				rend.draw_color((ii + jj) % 2 ? black : white);
				rend.fill_rect(Rect(ii*TILE_WIDTH, jj*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
			}

		// draw pawn:
		rend.render("pawn", Rect(_px*TILE_WIDTH, _py*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

		logi() << "<< rending scene done.";
	}

	virtual void on_key_event(const sally::keyboard_event& event_, sally::Window* win_) {
		int oldpx = _px, oldpy = _py;
		if (event_._type == sally::keyboard_event::KEY_PRESSED)
			switch (event_._keycode) {
			case SDLK_KP_8:
			case SDLK_UP:
				_py = inc(_py, -1, 0, BOARD_HEIGHT);
				break;
			case SDLK_KP_2:
			case SDLK_DOWN:
				_py = inc(_py, +1, 0, BOARD_HEIGHT);
				break;
			case SDLK_KP_4:
			case SDLK_LEFT:
				_px = inc(_px, -1, 0, BOARD_WIDTH);
				break;
			case SDLK_KP_6:
			case SDLK_RIGHT:
				_px = inc(_px, +1, 0, BOARD_WIDTH);
				break;
			case SDLK_ESCAPE:
				sally::System::request_shutdown();
				break;
			}
		if (_px != oldpx || _py != oldpy)
			if (win_)
				win_->invalidate();
			else
				sally::loge() << "key event with no window?!";
	}

private:
	static int inc(int v_, int d_, int l_, int h_) { v_ += d_; return v_<l_ ? l_ : (v_>=h_ ? h_ - 1 : v_); }

	int _px, _py;
};
