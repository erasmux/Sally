#pragma once

#include <sally/common.hpp>
#include <sally/gfx.hpp>

namespace sally {

	class keyboard_event_handler;
	class mouse_button_event_handler;
	class mouse_motion_event_handler;

	class step_event_handler {
	public:
		virtual void on_step_event() = 0;
		virtual ~step_event_handler() = default;
	};

	class System {
	public:
		class InitGuard {
		public:
			InitGuard();
			~InitGuard();
		};

		static void main_loop();

		// pushes the given user event. The uev_ if non-null will be deleted after
		// the event is processed (by the System), the udata_ is passed as is.
		// returns false on failure (i.e. queue full).
		static bool push_event(int code_, void* data1_ = nullptr, void* data2_ = nullptr) {
			return push_event(USER_EVENT_DELTA, code_, data1_, data2_);
		}

		static void request_shutdown();
		static void request_render();

		// the step event is called right before rendering a frame
		static step_event_handler* set_step_event_handler(step_event_handler* handler_) {
			// assumes will not be called concurrently from different threads
			step_event_handler* old_handler = _step_event_handler;
			_step_event_handler = handler_;
			return old_handler;
		}

		static keyboard_event_handler* set_keyboard_event_handler(keyboard_event_handler* handler_) {
			// assumes will not be called concurrently from different threads
			keyboard_event_handler* old_handler = _keyboard_event_handler;
			_keyboard_event_handler = handler_;
			return old_handler;
		}

		static mouse_button_event_handler* set_mouse_button_event_handler(mouse_button_event_handler* handler_) {
			// assumes will not be called concurrently from different threads
			mouse_button_event_handler* old_handler = _mouse_button_event_handler;
			_mouse_button_event_handler = handler_;
			return old_handler;
		}

		static mouse_motion_event_handler* set_mouse_motion_event_handler(mouse_motion_event_handler* handler_) {
			// assumes will not be called concurrently from different threads
			mouse_motion_event_handler* old_handler = _mouse_motion_event_handler;
			_mouse_motion_event_handler = handler_;
			return old_handler;
		}

		static WindowManager& window_manger() { return _window_mgr; }
		static FontManager& font_manger() { return _font_mgr; }
		static std::string resouce_path(const char* rel_path_);

	private:
		System();
		friend class InitGuard;

		static void init();
		static void destroy();
		static void wakeup() { push_event(WAKEUP_EVENT_DELTA, 0, nullptr, nullptr);	}
		static bool push_event(unsigned int type_delta_, int code_, void* data1_, void* data2_);
		static bool handle_event(const SDL_Event& ev_);

		enum { USER_EVENT_DELTA=0, WAKEUP_EVENT_DELTA, FRAME_EVENT_DELTA, USER_EVENTS_TOTAL };

		static WindowManager _window_mgr;
		static FontManager _font_mgr;
		static step_event_handler* _step_event_handler;
		static keyboard_event_handler* _keyboard_event_handler;
		static mouse_button_event_handler* _mouse_button_event_handler;
		static mouse_motion_event_handler* _mouse_motion_event_handler;
		static std::string _resource_base;
		static unsigned int _user_event_base;
	};

}
