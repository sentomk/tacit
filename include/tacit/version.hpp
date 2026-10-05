#pragma once

#include <string_view>

namespace tacit {

[[nodiscard]] auto version() noexcept -> std::string_view;

} // namespace tacit
