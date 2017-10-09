#pragma once

#include <sally/sally.hpp>

class sliding_pawn : public sally::keyboard_event_handler, public sally::RenderProvider
{
public:
	static const int BOARD_WIDTH = 8;
	static const int BOARD_HEIGHT = 8;
	static const int START_POS_X = 3;
	static const int START_POS_Y = 6;

	static const int TILE_WIDTH = 64;
	static const int TILE_HEIGHT = TILE_WIDTH;
	static const int FONT_SIZE = 18;
	static const int BOARD_BORDER = 1;
	static const int BOARD_PADDING = 10 * FONT_SIZE - 2 * BOARD_BORDER;
	static const int BOARD_PIXEL_WIDTH = 2 * BOARD_BORDER + BOARD_WIDTH * TILE_WIDTH;
	static const int WINDOW_WIDTH = BOARD_PADDING + BOARD_PIXEL_WIDTH;
	static const int WINDOW_HEIGHT = 2 * BOARD_BORDER + BOARD_HEIGHT * TILE_HEIGHT;

	sliding_pawn()
		: _px(3), _py(6)
	{}

	virtual void initialize(sally::Window& win_) {
		using namespace sally;

		static const Color text_color{ 0,   0,   0 };

		sally::Font* fnt = System::font_manger().load_font("sample", System::resouce_path("examples/SlidingPawn/sample.ttf"), FONT_SIZE);
		win_.renderer().load_image("pawn", System::resouce_path("examples/SlidingPawn/chess_pawn.png"));
		win_.renderer().insert("pawn_pos_title", new TextLine(fnt, text_color, "Pawn position:"));
		win_.renderer().insert("pawn_pos_x", new TextLine(fnt, text_color, ""));
		win_.renderer().insert("pawn_pos_y", new TextLine(fnt, text_color, ""));
		win_.renderer().insert("pawn_pos_an", new TextLine(fnt, text_color, ""));
		update_pawn_position_labels(win_);
	}

	virtual void render(sally::Window& win_) {
		using namespace sally;

		static const Color white{ 255, 255, 255 };
		static const Color black{   0,   0,   0 };
		static const Color padding{ 240, 240, 240 };

		logi() << ">> rending scene ...";

		Renderer& rend = win_.renderer();

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

				rend.render("pawn_pos_title", x0, y0);
				x0 += FONT_SIZE;
				y0 += skip;
				rend.render("pawn_pos_x", x0, y0);
				y0 += skip;
				rend.render("pawn_pos_y", x0, y0);
				y0 += skip;
				rend.render("pawn_pos_an", x0, y0);
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

		// draw pawn:
		rend.render("pawn", Rect(x0+_px*TILE_WIDTH, y0+_py*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

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
			if (win_) {
				update_pawn_position_labels(*win_);
				win_->invalidate();
			}
			else
				sally::loge() << "key event with no window?!";
	}

	void update_pawn_position_labels(sally::Window& win_) {
		sally::Renderer& rend = win_.renderer();

		std::ostringstream ost;
		ost << "x: " << _px;
		update_label_text(rend, "pawn_pos_x", ost.str());

		ost.swap(std::ostringstream()); // reset ost
		ost << "y: " << _py;
		update_label_text(rend, "pawn_pos_y", ost.str());

		ost.swap(std::ostringstream()); // reset ost
		ost << "AN: " << static_cast<char>('a'+_px) << static_cast<char>('1'+_py);
		update_label_text(rend, "pawn_pos_an", ost.str());
	}

	void update_label_text(sally::Renderer& rend_, const char* label_name_, const std::string& text_)
	{
		if (auto label = dynamic_cast<sally::TextLine*>(rend_.lookup(label_name_)))
			label->set_text(text_);
	}

private:
	static int inc(int v_, int d_, int l_, int h_) { v_ += d_; return v_<l_ ? l_ : (v_>=h_ ? h_ - 1 : v_); }

	int _px, _py;
};
