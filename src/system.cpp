#include <sally/system.hpp>
#include <sally/util/logger.hpp>
#include <sally/input/input_events.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iomanip>

namespace sally {

	//static
	WindowManager System::_window_mgr;
	//static
	FontManager System::_font_mgr;
	//static
	step_event_handler* System::_step_event_handler;
	//static
	keyboard_event_handler* System::_keyboard_event_handler;
	//static
	mouse_button_event_handler* System::_mouse_button_event_handler;
	//static
	mouse_motion_event_handler* System::_mouse_motion_event_handler;
	//static
	std::string System::_resource_base = "../../../"; // TODO: This value is good for sally examples but obviously should NOT be hard-coded here!
	//static
	unsigned int System::_user_event_base = 0;

	SDL_atomic_t system_init_count = { 0 };
	SDL_atomic_t system_shutdown_pending = { 0 };

	// we don't really support concurrent inits
	// additionally, a full solution will allow having init guards on subsystems (i.e. video, audio, etc.)
	System::InitGuard::InitGuard() {
		if (SDL_AtomicIncRef(&system_init_count) == 0)
			System::init();
	}

	System::InitGuard::~InitGuard() {
		if (SDL_AtomicDecRef(&system_init_count))
			System::destroy();
	}

	void System::init() {
		bool sdl_inited = false, img_inited = false, ttf_inited = false;

		try {
			SDL_SetMainReady();

			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
				throw sdl_exception("SDL_Init failed");
			sdl_inited = true;

			_user_event_base = SDL_RegisterEvents(USER_EVENTS_TOTAL);
			if (_user_event_base == (unsigned)-1)
				throw sdl_exception("SDL_RegisterEvents failed");

			if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
				throw sdl_exception("IMG_Init failed");
			img_inited = true;

			if (TTF_Init() != 0)
				throw sdl_exception("TTF_Init failed");
			ttf_inited = true;
		}
		catch (...) {
			if (ttf_inited)
				TTF_Quit();
			if (img_inited)
				IMG_Quit();
			if (sdl_inited)
				SDL_Quit();
			throw;
		}
	}

	void System::destroy() {
		font_manger().clear();
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
	}

	//static
	std::string System::resouce_path(const char* rel_path_)
	{
		return std::string(_resource_base) + rel_path_;
	}

	//static
	void System::request_shutdown() {
		SDL_AtomicSet(&system_shutdown_pending, 1);
		wakeup();
	}

	//static
	void System::request_render() {
		wakeup();
	}

	//static
	bool System::push_event(unsigned int type_delta_, int code_, void* data1_, void* data2_)
	{
		SDL_Event ev;
		SDL_zero(ev);
		ev.type = _user_event_base + type_delta_;
		ev.user.code = code_;
		ev.user.data1 = data1_;
		ev.user.data2 = data2_;
		return SDL_PushEvent(&ev) >= 0;
	}


	void debug_sdl_event(const SDL_Event& e, unsigned int user_event_base_, unsigned int user_event_count_) {
		auto msg = logi();
		msg << "event " << std::hex << e.type << std::dec << " : ";
		switch (e.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			msg << " key " << (int)e.key.keysym.sym
				<< " '" << (char)e.key.keysym.sym << "'"
				<< " (s " << e.key.keysym.scancode << " m " << e.key.keysym.mod << ")"
				<< " window " << (int)e.key.windowID << " state " << (int)e.key.state
				<< " repeat " << (int)e.key.repeat;
			break;
		case SDL_TEXTINPUT:
			msg << " text \"" << e.text.text << "\"";
			break;
		case SDL_TEXTEDITING:
		{
			char txt[SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
			char* p = txt;
			for (int i = 0; i < e.edit.length; ++i)
				*p++ = e.edit.text[(e.edit.start + i) % SDL_TEXTEDITINGEVENT_TEXT_SIZE];
			*p = 0;
			msg << " edit \"" << txt << "\" (" << e.edit.length << ")";
		}
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			msg << " mouse " << e.button.which << " button " << (int)e.button.button
				<< " window " << (int)e.button.windowID << " state " << (int)e.button.state
				<< " @ (" << e.button.x << "," << e.button.y << ")";
			break;
		case SDL_MOUSEMOTION:
			msg << " mouse " << e.motion.which
				<< " d (" << e.motion.xrel << "," << e.motion.yrel << ")"
				<< " window " << (int)e.motion.windowID << " state " << (int)e.motion.state
				<< " @ (" << e.motion.x << "," << e.motion.y << ")";
			break;
		case SDL_WINDOWEVENT:
			msg << " window " << (int)e.window.windowID
				<< " ev " << (int)e.window.event
				<< " data1 " << e.window.data1 << " data2 " << e.window.data2;
			break;
		default:
			if (e.type >= user_event_base_ && e.type < user_event_base_ + user_event_count_)
			{
				msg << " user event " << e.type - user_event_base_
					<< " code " << e.user.type
					<< " user event object " << e.user.data1
					<< " user data " << e.user.data2;
			}
		}

	}

	//static
	void System::main_loop()
	{
		static const Uint32 MIN_FRAME_MS = 5; // if "drawing" a frame takes less than MIN_FRAME_MS millisecond we will delay
		// notice this should only happend on frames which did not actually draw

		logi() << "starting main event loop...";
		bool quit = false;
		while (!quit)
		{
			Uint32 frame_start_tick = SDL_GetTicks();

			// first handle "real" events:
			SDL_Event ev;
			while (!quit && SDL_PollEvent(&ev))
				quit = handle_event(ev);

			// right before drawing a frame generate a step event
			if (!quit && _step_event_handler)
				_step_event_handler->on_step_event();

			if (!quit) {
				// finally draw whatever is needed:
				_window_mgr.render_all_pending();

				// if necessary delay until next frame:
				while (SDL_GetTicks() - frame_start_tick < MIN_FRAME_MS)
					SDL_Delay(1);
			}
		}
	}

	// static
	bool System::handle_event(const SDL_Event& ev_) {
		try {
			//debug_sdl_event(ev,_user_event_base,USER_EVENTS_TOTAL);

			bool quit = ev_.type == SDL_QUIT || SDL_AtomicGet(&system_shutdown_pending) != 0;
			bool terminate = ev_.type == SDL_APP_TERMINATING;
			if (quit || terminate)
			{
				if (quit)
					logi() << "received quit request, exiting main loop";
				else
					logi() << "received terminate request, exiting main loop";
				return true;
			}
			else if (quit)
				logi() << "ignoring quit request";
			else if (terminate)
				logi() << "ignoring terminate request";

			switch (ev_.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if (_keyboard_event_handler)
					_keyboard_event_handler->on_key_event(
						keyboard_event(
							static_cast<keyboard_event::event_type>(ev_.type),
							ev_.key.timestamp,
							ev_.key.repeat,
							ev_.key.keysym.scancode,
							ev_.key.keysym.sym,
							ev_.key.keysym.mod
						),
						_window_mgr.window_by_id(ev_.key.windowID)
					);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (_mouse_button_event_handler)
					_mouse_button_event_handler->on_mousebutton__event(
						mouse_button_event(
							static_cast<mouse_button_event::event_type>(ev_.type),
							ev_.button.timestamp,
							ev_.button.which,
							static_cast<mouse_button_event::button_type>(ev_.button.button),
							ev_.button.clicks
						),
						_window_mgr.window_by_id(ev_.button.windowID),
						ev_.button.x,
						ev_.button.y
					);
				break;
			case SDL_MOUSEMOTION:
				if (_mouse_motion_event_handler)
					_mouse_motion_event_handler->on_mouse_motion_event(
						mouse_motion_event(
							ev_.motion.timestamp,
							ev_.motion.which,
							static_cast<mouse_motion_event::button_mask_type>(ev_.motion.state)
						),
						_window_mgr.window_by_id(ev_.motion.windowID),
						ev_.motion.x,
						ev_.motion.y,
						ev_.motion.xrel,
						ev_.motion.yrel
					);
				break;
			}
		}
		catch (sally::exception& e) {
			loge() << typeid(e).name() << " while hanlding event type " << ev_.type
				<< " : " << e.what();
		}
		catch (std::exception& e) {
			loge() << typeid(e).name() << " while hanlding event type " << ev_.type
				<< " : " << e.what();
		}
		catch (...) {
			loge() << "unknown exception while hanlding event type " << ev_.type;
		}
		return false;
	}

}
