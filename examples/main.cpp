#include <tacit/version.hpp>

#include <iostream>

auto main() -> int { std::cout << "tacit " << tacit::version() << '\n'; }
