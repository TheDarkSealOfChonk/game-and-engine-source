#include <stdexcept>
#include <iostream>
#include <Windows.h>
#include <string>
#include <locale>
#include <codecvt>
#include "war_sim.hpp"
#undef NDEBUG

int main(int argc, char** argv) {
	te::TheEngine app{};
	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
