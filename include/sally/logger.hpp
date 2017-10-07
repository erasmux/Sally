#pragma once

#include <sally/common.hpp>
#include <ostream>
#include <sstream>

namespace Sally {

	class generic_logger {
	public:
		class message;

		// helper functions to start building a message with the appropriate prefix
		message info();
		message warn();
		message error();
		message fatal();

		// newline is automatically added at end of message
		virtual void output(const char* msg_) = 0;
		// newline is automatically added at end of message
		virtual void output(const std::string& msg_) = 0;

		// active_logger is guaranteed to return a valid logger object
		static generic_logger& active_logger();

		// set_active_logger replaces the previous logger
		// or resets the default NullLogger if nullptr is given as the parameter.
		// setting the active logger from multiple threads in parallel is not thread safe.
		static void set_active_logger(const shared_ptr<generic_logger> logger_) { _active = logger_; }

	private:
		static shared_ptr<generic_logger> _active;

		message msg_with_header(const char* type_);
	};

	// helper class for building and logging a message
	class generic_logger::message {
	public:
		message(generic_logger& logger_) : _logger(logger_) {}
		message(message&& other_) : _ost(std::move(other_._ost)), _logger(other_._logger)
			{ other_._ost.setstate(std::ios::badbit); } // prevent other_ dtor from logging empty line

		template<typename T>
		message& operator<<(const T& x_) { _ost << x_; return *this; }

		~message() { if (_ost) _logger.output(_ost.str()); }
	private:
		std::ostringstream _ost;
		generic_logger& _logger;
	};

	// NullLogger totally ignores all messages
	class null_logger : public generic_logger {
	public:
		virtual void output(const char* msg_);
		virtual void output(const std::string& msg_);

		static null_logger& instance() { return _instance; }
	private:
		static null_logger _instance;
	};

	class stream_logger : public generic_logger {
	public:
		stream_logger(const shared_ptr<std::ostream>& os_) : _os(os_) {}

		virtual void output(const char* msg_);
		virtual void output(const std::string& msg_);
	private:
		shared_ptr<std::ostream> _os;
	};

	class console_logger : public generic_logger {
	public:
		virtual void output(const char* msg_);
		virtual void output(const std::string& msg_);
	};

	//static
	inline generic_logger& generic_logger::active_logger() { return _active ? *_active : null_logger::instance(); }

	// convience functions for logging to active logger:
	inline generic_logger::message logi() { return generic_logger::active_logger().info(); }
	inline generic_logger::message logw() { return generic_logger::active_logger().warn(); }
	inline generic_logger::message loge() { return generic_logger::active_logger().error(); }
	inline generic_logger::message logf() { return generic_logger::active_logger().fatal(); }
};
