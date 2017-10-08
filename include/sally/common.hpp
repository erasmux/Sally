#pragma once

#include <string>
#include <cstring>
#include <memory>

#ifdef _WIN32
# define SALLY_WINDOWS
#endif

#ifdef SALLY_WINDOWS
# define SALLY_NOEXCEPT throw()
#else
# define SALLY_NOEXCEPT noexcept
#endif

#ifdef SALLY_WINDOWS
# define SALLY_SNFORMAT(buf,size,f,...) _snprintf_s(buf,size,f,__VA_ARGS__)
# define SALLY_SFORMAT(buf,f,...) _snprintf_s(buf,sizeof(buf),f,__VA_ARGS__)
#else
# define SALLY_SNFORMAT(buf,size,f,...) snprintf(buf,size,f,__VA_ARGS__)
# define SALLY_SFORMAT(buf,f,...) sprintf(buf,sizeof(buf),f,__VA_ARGS__)
#endif

#ifdef SALLY_WINDOWS
# define SALLY_GMTIME(timer,res) gmtime_s(res,timer)
#else
# define SALLY_GMTIME(timer,res) gmtime_r(timer,res)
#endif

namespace sally {

	using std::unique_ptr;
	using std::shared_ptr;
	using std::make_shared;

	inline size_t strlen_s(const char* str_) { using namespace std;  return str_ ? strlen(str_) : 0; }

	class exception {
	public:
		virtual ~exception() {}
		virtual const char* what() const SALLY_NOEXCEPT;
	};

	class general_exception : public exception {
	public:
		general_exception(const char* what_) : _what(what_) {}
		virtual const char* what() const SALLY_NOEXCEPT;
	private:
		std::string _what;
	};

	class sdl_exception : public exception {
	public:
		sdl_exception(const char* msg_, const char* arg1_ = 0, const char* arg2_ = 0, const char* arg3_ = 0);
		virtual const char* what() const SALLY_NOEXCEPT;
	private:
		std::string _what;
	};

	class img_exception : public exception {
	public:
		img_exception(const char* msg_, const char* arg1_ = 0, const char* arg2_ = 0, const char* arg3_ = 0);
		virtual const char* what() const SALLY_NOEXCEPT;
	private:
		std::string _what;
	};

	class ttf_exception : public exception {
	public:
		ttf_exception(const char* msg_, const char* arg1_ = 0, const char* arg2_ = 0, const char* arg3_ = 0);
		virtual const char* what() const SALLY_NOEXCEPT;
	private:
		std::string _what;
	};

}
