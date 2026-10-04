#pragma once

#include <string_view>

namespace zk_snark {

[[nodiscard]] auto version() noexcept -> std::string_view;

} // namespace zk_snark
