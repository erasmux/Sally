#include <sally/logger.hpp>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <SDL.h>

namespace Sally {

	//static
	Logger Logger::_instance;

	auto Logger::info() -> Msg
	{
		return msg_with_header("info");
	}
	
	auto Logger::warn() -> Msg
	{
		return msg_with_header("warn");
	}
	
	auto Logger::error() -> Msg
	{
		return msg_with_header("error");
	}
	
	auto Logger::fatal() -> Msg
	{
		return msg_with_header("fatal");
	}

	auto Logger::msg_with_header(const char* type_) -> Msg
	{
		using namespace std;
		Logger::Msg msg(*this);
		time_t t = time(0);
		struct tm now;
		SALLY_GMTIME(&t, &now);
		char buf[128];
		SALLY_SFORMAT(buf, sizeof(buf), "[%s][%02d/%02d/%02d %02d:%02d:%02d] ", type_,
			now.tm_mday, now.tm_mon + 1, now.tm_year % 100, now.tm_hour, now.tm_min, now.tm_sec);
		msg << buf;
		return msg;
	}

	void Logger::output(const char* msg_)
	{
		std::cout << msg_ << std::endl;
	}

	void Logger::output(const std::string& msg_)
	{
		std::cout << msg_ << std::endl;
	}

}
