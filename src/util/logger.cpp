#include <sally/util/logger.hpp>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <SDL.h>

namespace sally {

	//static
	shared_ptr<generic_logger> generic_logger::_active;

	auto generic_logger::info() -> message
	{
		return msg_with_header("info");
	}
	
	auto generic_logger::warn() -> message
	{
		return msg_with_header("warn");
	}
	
	auto generic_logger::error() -> message
	{
		return msg_with_header("error");
	}
	
	auto generic_logger::fatal() -> message
	{
		return msg_with_header("fatal");
	}

	auto generic_logger::msg_with_header(const char* type_) -> message
	{
		using namespace std;
		message msg(*this);
		time_t t = time(0);
		struct tm now;
		SALLY_GMTIME(&t, &now);
		char buf[128];
		SALLY_SFORMAT(buf, sizeof(buf), "[%s][%02d/%02d/%02d %02d:%02d:%02d] ", type_,
			now.tm_mday, now.tm_mon + 1, now.tm_year % 100, now.tm_hour, now.tm_min, now.tm_sec);
		msg << buf;
		return msg;
	}

	//static
	null_logger null_logger::_instance;

	void null_logger::output(const char* msg_)
	{
	}

	void null_logger::output(const std::string& msg_)
	{
	}

	void stream_logger::output(const char* msg_)
	{
		*_os << msg_ << std::endl;
	}

	void stream_logger::output(const std::string& msg_)
	{
		*_os << msg_ << std::endl;
	}

	void console_logger::output(const char* msg_)
	{
		std::cout << msg_ << std::endl;
	}

	void console_logger::output(const std::string& msg_)
	{
		std::cout << msg_ << std::endl;
	}

}
