#pragma once

#include <sally/sally.hpp>
#include <cmath>

class sliding_pawn :
	public::sally::step_event_handler,
	public sally::keyboard_event_handler,
	public sally::RenderProvider
{
public:
	static const int BOARD_WIDTH = 8;
	static const int BOARD_HEIGHT = 8;
	static const int PLYR_START_POS_X = 3;
	static const int PLYR_START_POS_Y = 6;
	static const int OPP_START_POS_X = BOARD_WIDTH-1 - PLYR_START_POS_X;
	static const int OPP_START_POS_Y = BOARD_HEIGHT-1 - PLYR_START_POS_Y;
	static const int AI_MOVE_INTERVAL_MS = 300;

	static const int TILE_WIDTH = 64;
	static const int TILE_HEIGHT = TILE_WIDTH;
	static const int FONT_SIZE = 18;
	static const int BOARD_BORDER = 1;
	static const int BOARD_PADDING = 14 * FONT_SIZE - 2 * BOARD_BORDER;
	static const int BOARD_PIXEL_WIDTH = 2 * BOARD_BORDER + BOARD_WIDTH * TILE_WIDTH;
	static const int WINDOW_WIDTH = BOARD_PADDING + BOARD_PIXEL_WIDTH;
	static const int WINDOW_HEIGHT = 2 * BOARD_BORDER + BOARD_HEIGHT * TILE_HEIGHT;

	sliding_pawn() :
		_px(PLYR_START_POS_X),
		_py(PLYR_START_POS_Y),
		_ox(OPP_START_POS_X),
		_oy(OPP_START_POS_Y),
		_plyr_wins(0),
		_next_ai_move(0),
		_win(nullptr)
	{}

	virtual void initialize(sally::Window& win_) {
		using namespace sally;

		static const Color text_color{ 0,   0,   0 };

		_win = &win_;

		sally::Font* fnt = System::font_manger().load_font("sample", System::resouce_path("examples/SlidingPawn/sample.ttf"), FONT_SIZE);
		win_.renderer().load_image("white_pawn", System::resouce_path("examples/SlidingPawn/white_pawn.png"));
		win_.renderer().load_image("black_pawn", System::resouce_path("examples/SlidingPawn/black_pawn.png"));
		win_.renderer().insert("plyr_wins", new TextLine(fnt, text_color, ""));
		win_.renderer().insert("plyr_pos_title", new TextLine(fnt, text_color, "Player (white) position:"));
		win_.renderer().insert("plyr_pos", new TextLine(fnt, text_color, ""));
		win_.renderer().insert("opp_pos_title", new TextLine(fnt, text_color, "Opponent (black) position:"));
		win_.renderer().insert("opp_pos", new TextLine(fnt, text_color, ""));
		update_pawn_position_label("plyr_pos", _px, _py);
		update_pawn_position_label("opp_pos", _ox, _oy);
		update_wins_label();

		_next_ai_move = clock_tick() + AI_MOVE_INTERVAL_MS;
	}

	virtual void render(sally::Window& win_) {
		using namespace sally;

		static const Color white{ 255, 255, 255 };
		static const Color black{   0,   0,   0 };
		static const Color padding{ 240, 240, 240 };

		Renderer& rend = win_.renderer();

		logi() << ">> rending scene ...";

		if (BOARD_PADDING)
		{
			// fill padding:
			rend.draw_color(padding);
			Rect area = rend.output_rect();
			area._width = BOARD_PADDING;
			rend.fill_rect(area);

			// render text:
			if (Font* fnt = System::font_manger().lookup("sample"))
			{
				int skip = fnt->lineskip();
				int x0 = FONT_SIZE;
				int y0 = skip;

				rend.render("plyr_wins", x0, y0);
				y0 += 2 * skip;
				rend.render("plyr_pos_title", x0, y0);
				x0 += FONT_SIZE;
				y0 += skip;
				rend.render("plyr_pos", x0, y0);
				x0 -= FONT_SIZE;
				y0 += 2 * skip;
				rend.render("opp_pos_title", x0, y0);
				x0 += FONT_SIZE;
				y0 += skip;
				rend.render("opp_pos", x0, y0);
			}
		}

		// draw board by first filling board+border white and then drawing black tiles:
		rend.draw_color(white);
		Rect area = rend.output_rect();
		area._x = area._width - BOARD_PIXEL_WIDTH;
		area._width = BOARD_PIXEL_WIDTH;
		rend.fill_rect(area);
		rend.draw_color(black);
		scalar x0 = area._x + BOARD_BORDER, y0 = BOARD_BORDER;
		for (int ii = 0; ii < BOARD_WIDTH; ++ii)
			for (int jj = 0; jj < BOARD_HEIGHT; ++jj)
			if ((ii + jj) % 2)
				rend.fill_rect(Rect(x0+ii*TILE_WIDTH, y0+jj*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

		// draw pawns:
		rend.render("black_pawn", Rect(x0 + _ox*TILE_WIDTH, y0 + _oy*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
		rend.render("white_pawn", Rect(x0 + _px*TILE_WIDTH, y0 + _py*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

		logi() << "<< rending scene done.";
	}

	virtual void on_step_event() {
		using namespace sally;

		if (clock_tick() >= _next_ai_move) {
			_next_ai_move += AI_MOVE_INTERVAL_MS;
			logi() << "moving opponent...";
			move_opponent();
		}
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
		{
			if (!check_and_handle_game_reset())
				update_pawn_position_label("plyr_pos", _px, _py);
			_win->invalidate();
		}
	}

	bool check_and_handle_game_reset() {
		if (_px != _ox || _py != _oy)
			return false;

		sally::logi() << "player wins :)";

		++_plyr_wins;
		// reset game:
		_px = PLYR_START_POS_X;
		_py = PLYR_START_POS_Y;
		_ox = OPP_START_POS_X;
		_oy = OPP_START_POS_Y;
		// update labels:
		update_pawn_position_label("plyr_pos", _px, _py);
		update_pawn_position_label("opp_pos", _ox, _oy);
		update_wins_label();
		return true;
	}

	void update_wins_label() {
		std::ostringstream ost;
		ost << "wins: " << _plyr_wins;
		update_label_text("plyr_wins", ost.str());
	}

	void update_pawn_position_label(const char* label_name_, int x_, int y_) {
		std::ostringstream ost;
		ost << static_cast<char>('a' + x_) << static_cast<char>('1' + y_);
		ost << " (" << x_ << "," << y_ << ")";
		update_label_text(label_name_, ost.str());
	}

	void update_label_text(const char* label_name_, const std::string& text_) {
		if (auto label = dynamic_cast<sally::TextLine*>(_win->renderer().lookup(label_name_)))
			label->set_text(text_);
	}

	void move_opponent() {
		int sx = 0, sy = 0;

		if (_ox == 0)
			sx = -_px;
		else if (_ox == BOARD_WIDTH - 1)
			sx = BOARD_HEIGHT - _px;
		else
			sx = _px - _ox;

		if (_oy == 0)
			sy = -_py;
		else if (_oy == BOARD_HEIGHT - 1)
			sy = BOARD_HEIGHT - _py;
		else
			sy = _py - _oy;

		if (std::abs(sx) > std::abs(sy))
			_oy += (sy > 0 || sy == 0 && _oy >= BOARD_HEIGHT / 2) ? -1 : 1;
		else
			_ox += (sx > 0 || sx == 0 && _ox >= BOARD_WIDTH / 2) ? -1 : 1;

		if (!check_and_handle_game_reset())
			update_pawn_position_label("opp_pos", _ox, _oy);
		_win->invalidate();
	}

private:
	static int inc(int v_, int d_, int l_, int h_) { v_ += d_; return v_<l_ ? l_ : (v_>=h_ ? h_ - 1 : v_); }

	int _px, _py, _ox, _oy, _plyr_wins;
	sally::ticks_t _next_ai_move;
	sally::Window* _win;
};
