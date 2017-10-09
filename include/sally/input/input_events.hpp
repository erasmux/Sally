#pragma once

#include <sally/common.hpp>
#include <SDL_events.h>

namespace sally {

	class Window;

	struct keyboard_event {
		enum event_type {
			KEY_PRESSED = SDL_KEYDOWN,
			KEY_RELEASED = SDL_KEYUP
		};

		static const uint32_t TOUCH_MOUSE_ID = SDL_TOUCH_MOUSEID;

		event_type _type;    // button pressed or released
		ticks_t _tick;    // timestamp of the event in clock ticks (ms)
		int _repeat;         // non-zero if this is a key repeat
		SDL_Scancode _scancode; // SDL_Scancode of key pressed/released
		SDL_Keycode _keycode;   // SDL_Keycode of key pressed/released
		Uint16 _keymod;         // SDL_Keymod of current key modifiers

		keyboard_event(event_type type_, ticks_t tick_, int repeat_, SDL_Scancode scancode_, SDL_Keycode keycode_, Uint16 keymod_)
			: _type(type_), _tick(tick_), _repeat(repeat_), _scancode(scancode_), _keycode(keycode_), _keymod(keymod_)
		{}

		// a few common modifiers (for full list see SDL documentation)
		bool ctrl_mod() const { return _keymod & KMOD_CTRL; }
		bool shift_mod() const { return _keymod & KMOD_SHIFT; }
		bool alt_mod() const { return _keymod & KMOD_ALT; }
	};

	class keyboard_event_handler {
	public:
		virtual void on_key_event(const keyboard_event& event_, Window* win_) = 0;

		virtual ~keyboard_event_handler() = default;
	};

	struct mouse_button_event {
		enum event_type {
			BUTTON_PRESSED = SDL_MOUSEBUTTONDOWN,
			BUTTON_RELEASED = SDL_MOUSEBUTTONUP
		};

		enum button_type {
			BUTTON_LEFT = SDL_BUTTON_LEFT,
			BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,
			BUTTON_RIGHT = SDL_BUTTON_RIGHT,
			BUTTON_X1 = SDL_BUTTON_X1,
			BUTTON_X2 = SDL_BUTTON_X2
		};

		static const uint32_t TOUCH_MOUSE_ID = SDL_TOUCH_MOUSEID;

		event_type _type;    // button pressed or released
		ticks_t _tick;    // timestamp of the event in clock ticks (ms)
		uint32_t _mouseid;   // id of mouse click or TOUCH_MOUSE_ID for touch input device
		button_type _button; // which button was pressed or released
		int _clicks;         // 1 for single-click, 2 for double-click, etc.

		mouse_button_event(event_type type_, ticks_t tick_, uint32_t mouseid_, button_type button_, int clicks_)
			: _type(type_), _tick(tick_), _mouseid(mouseid_), _button(button_), _clicks(clicks_)
		{}

		bool left_button_pressed() const { return _type == BUTTON_PRESSED && _button == BUTTON_LEFT; }
		bool middle_button_pressed() const { return _type == BUTTON_PRESSED && _button == BUTTON_MIDDLE; }
		bool right_button_pressed() const { return _type == BUTTON_PRESSED && _button == BUTTON_RIGHT; }
		bool x1_button_pressed() const { return _type == BUTTON_PRESSED && _button == BUTTON_X1; }
		bool x2_button_pressed() const { return _type == BUTTON_PRESSED && _button == BUTTON_X2; }

		bool left_button_released() const { return _type == BUTTON_RELEASED && _button == BUTTON_LEFT; }
		bool middle_button_released() const { return _type == BUTTON_RELEASED && _button == BUTTON_MIDDLE; }
		bool right_button_released() const { return _type == BUTTON_RELEASED && _button == BUTTON_RIGHT; }
		bool x1_button_released() const { return _type == BUTTON_RELEASED && _button == BUTTON_X1; }
		bool x2_button_released() const { return _type == BUTTON_RELEASED && _button == BUTTON_X2; }
	};

	class mouse_button_event_handler {
	public:
		virtual void on_mousebutton__event(const mouse_button_event& event_, Window* win_, scalar x_, scalar y_) = 0;

		virtual ~mouse_button_event_handler() = default;
	};

	struct mouse_motion_event {
		enum button_mask_type {
			BUTTON_LEFT = SDL_BUTTON_LMASK,
			BUTTON_MIDDLE = SDL_BUTTON_MMASK,
			BUTTON_RIGHT = SDL_BUTTON_RMASK,
			BUTTON_X1 = SDL_BUTTON_X1MASK,
			BUTTON_X2 = SDL_BUTTON_X2MASK
		};

		static const uint32_t TOUCH_MOUSE_ID = SDL_TOUCH_MOUSEID;

		ticks_t _tick;    // timestamp of the event in clock ticks (ms)
		uint32_t _mouseid;   // id of mouse click or TOUCH_MOUSE_ID for touch input device
		button_mask_type _pressed; // which button(s) are pressed

		mouse_motion_event(ticks_t tick_, uint32_t mouseid_, button_mask_type pressed_)
			: _tick(tick_), _mouseid(mouseid_), _pressed(pressed_)
		{}

		bool left_button_pressed() const { return _pressed & BUTTON_LEFT; }
		bool middle_button_pressed() const { return _pressed & BUTTON_MIDDLE; }
		bool right_button_pressed() const { return _pressed & BUTTON_RIGHT; }
		bool x1_button_pressed() const { return _pressed & BUTTON_X1; }
		bool x2_button_pressed() const { return  _pressed & BUTTON_X2; }
	};

	class mouse_motion_event_handler {
	public:
		virtual void on_mouse_motion_event(const mouse_motion_event& event_, Window* win_,
			scalar x_, scalar y_, scalar xrel_, scalar yrel_) = 0;

		virtual ~mouse_motion_event_handler() = default;
	};

}
