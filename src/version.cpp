#include <tacit/version.hpp>

namespace tacit {

auto version() noexcept -> std::string_view {
    return TACIT_VERSION;
}

} // namespace tacit
