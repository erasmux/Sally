#include "sliding_pawn.hpp"
#include <iostream>

template<typename Scenario, typename... Args>
int run(const Args&... args_)
{
}


int main(int argc, char **argv)
{
	using namespace sally;

	generic_logger::set_active_logger(make_shared<console_logger>());

	try {
		System::InitGuard initgrd;

		sliding_pawn scen;
		Window win("Sliding Pawn", sliding_pawn::WINDOW_WIDTH, sliding_pawn::WINDOW_HEIGHT, 0, &scen);
		System::set_event_handler(&scen);
		System::main_loop();
		System::set_event_handler(nullptr);
	}
	catch (sally::exception& e) {
		logf() << typeid(e).name() << " : " << e.what();
	}
	catch (std::exception& e) {
		logf() << typeid(e).name() << " : " << e.what();
	}
	catch (...) {
		logf() << "unknown exception";
	}

	return 0;
}
