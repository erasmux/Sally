#pragma once

#include <sally/common.hpp>
#include <ostream>
#include <sstream>

namespace Sally {

	class Logger {
	public:
		class Msg;

		Msg info();
		Msg warn();
		Msg error();
		Msg fatal();

		void output(const char* msg_);
		void output(const std::string& msg_);

		static Logger& instance() { return _instance;  }

	private:
		static Logger _instance;

		Msg msg_with_header(const char* type_);
	};

	class Logger::Msg {
	public:
		Msg(Logger& logger_) : _logger(logger_) {}
		Msg(Msg&& other_) : _ost(std::move(other_._ost)), _logger(other_._logger)
			{ other_._ost.setstate(std::ios::badbit); } // prevent other_ dtor from logging empty line

		template<typename T>
		Msg& operator<<(const T& x_) { _ost << x_; return *this; }

		~Msg() { if (_ost) _logger.output(_ost.str()); }
	private:
		std::ostringstream _ost;
		Logger& _logger;
	};

	inline Logger::Msg logi() { return Logger::instance().info(); }
	inline Logger::Msg logw() { return Logger::instance().warn(); }
	inline Logger::Msg loge() { return Logger::instance().error(); }
	inline Logger::Msg logf() { return Logger::instance().fatal(); }
};
