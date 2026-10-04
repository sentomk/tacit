#include <zk_snark/version.hpp>

#include <iostream>

auto main() -> int {
    std::cout << "zk-snark " << zk_snark::version()
              << "\nP0 scaffold: cryptographic primitives are not implemented yet.\n";
}
