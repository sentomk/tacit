#include <zk_snark/version.hpp>

namespace zk_snark {

auto version() noexcept -> std::string_view {
    return ZK_SNARK_VERSION;
}

} // namespace zk_snark
