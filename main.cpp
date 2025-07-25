#include "window.hpp"

#include <memory>





int main(int, char* [])
{
	auto window = std::make_unique<Window>();
	window->Loop();
	return EXIT_SUCCESS;
}