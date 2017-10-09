#pragma once

#include <sally/common.hpp>
#include <SDL_atomic.h>

struct SDL_mutex;

namespace sally {

	template<typename Lock>
	class generic_guard {
	public:
		generic_guard(Lock& lock_, bool initially_locked_=true)
			: _lock(lock_), _locked(initially_locked_)
		{ if (initially_locked_) _lock.lock(); }

		~generic_guard() { if (_locked) _lock.unlock();  }

		void lock() { if (!_locked) { _lock.lock(); _locked = true; } }
		void unlock() { if (_locked) { _lock.unlock(); _locked = false; } }
		bool try_lock() { return _locked || (_locked = _lock.try_lock());  }
	private:
		Lock& _lock;
		bool _locked;
	};

	class mutex {
	public:
		typedef generic_guard<mutex> Guard;

		mutex();
		~mutex();

		void lock();
		void unlock();
		bool try_lock();

	private:
		SDL_mutex* _mutex;
	};

	class spinlock {
	public:
		typedef generic_guard<spinlock> Guard;

		spinlock() { memset(&_lock, 0, sizeof(_lock)); }

		void lock() { SDL_AtomicLock(&_lock); }
		void unlock() { SDL_AtomicUnlock(&_lock); }
		bool try_lock() { return SDL_AtomicTryLock(&_lock) == SDL_TRUE; }

	private:
		SDL_SpinLock _lock;
	};

}
