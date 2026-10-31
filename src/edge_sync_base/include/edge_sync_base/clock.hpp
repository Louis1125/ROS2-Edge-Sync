#pragma once

#include <chrono>

namespace edge_sync::base {

using SteadyClock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;

}  // namespace edge_sync::base
