#include <tacit/version.hpp>

#include <iostream>

auto main() -> int {
    std::cout << "tacit " << tacit::version()
              << "\nP0 scaffold: cryptographic primitives are not implemented yet.\n";
}
