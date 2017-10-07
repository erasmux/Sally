#pragma once

#include <sally/common.hpp>
#include <SDL_events.h>

namespace Sally {

	class Window;

	class UserEventBase {
	public:
		virtual ~UserEventBase() = default;
	};

	class EventHandler {
	public:
		virtual void window_event(Window* win_, const SDL_WindowEvent& ev_) {}
		virtual void key_event(Window* win_, const SDL_KeyboardEvent& ev_) {}
		virtual void mouse_button(Window* win_, const SDL_MouseButtonEvent& ev_) {}
		virtual void mouse_moved(Window* win_, const SDL_MouseMotionEvent& ev_) {}
		virtual void frame_event() {} // called before drawing a frame
		virtual void user_event(int code_, UserEventBase* uev_, void* udata_) {}

		virtual bool quit(bool kill_) { return true; } // return false to ignore quit event

		virtual ~EventHandler() = default;
	};

}