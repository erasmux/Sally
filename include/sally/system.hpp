#pragma once

#include <sally/common.hpp>
#include <sally/gfx.hpp>

namespace sally {

	class EventHandler;
	class UserEventBase;

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
		static bool push_event(int code_, UserEventBase* uev_ = nullptr, void* udata_ = nullptr) {
			return push_event(USER_EVENT_DELTA, code_, uev_, udata_);
		}

		static void request_shutdown();
		static void request_render();

		static EventHandler* set_event_handler(EventHandler* handler_) {
			// assumes will only be called by main thread
			EventHandler* old_handler = _event_handler;
			_event_handler = handler_;
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
		static bool push_event(unsigned int type_delta_, int code_, UserEventBase* uev_, void* udata_);
		static bool handle_event(const SDL_Event& ev_);

		enum { USER_EVENT_DELTA=0, WAKEUP_EVENT_DELTA, FRAME_EVENT_DELTA, USER_EVENTS_TOTAL };

		static WindowManager _window_mgr;
		static FontManager _font_mgr;
		static EventHandler* _event_handler;
		static std::string _resource_base;
		static unsigned int _user_event_base;
	};

}