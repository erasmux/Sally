#include "SlidingPawn.hpp"
#include <iostream>

template<typename Scenario, typename... Args>
int run(const Args&... args_)
{
}


int main(int argc, char **argv)
{
	using namespace Sally;

	try {
		System::InitGuard initgrd;

		SlidingPawn scen;
		Window win("Sliding Pawn", SlidingPawn::WINDOW_WIDTH, SlidingPawn::WINDOW_HEIGHT, 0, &scen);
		System::set_event_handler(&scen);
		System::main_loop();
		System::set_event_handler(nullptr);
	}
	catch (Sally::exception& e) {
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
