#pragma once

#include <sally/threading.hpp>
#include <SDL_mutex.h>

namespace sally {

	// Mutex:

	mutex::mutex() : _mutex(SDL_CreateMutex())
	{
		if (!_mutex)
			throw sdl_exception("SDL_CreateMutex failed");
	}

	mutex::~mutex()
	{
		SDL_DestroyMutex(_mutex);
	}

	void mutex::lock()
	{
		if (SDL_LockMutex(_mutex) != 0)
			throw sdl_exception("SDL_LockMutex failed");
	}

	void mutex::unlock()
	{
		if (SDL_UnlockMutex(_mutex) != 0)
			throw sdl_exception("SDL_UnlockMutex failed");
	}
	
	bool mutex::try_lock()
	{
		int s = SDL_TryLockMutex(_mutex);
		if (s == 0)
			return true;
		if (s == SDL_MUTEX_TIMEDOUT)
			return false;
		throw sdl_exception("SDL_TryLockMutex failed");
	}

}
