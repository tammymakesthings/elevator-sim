//////////////////////////////////////////////////////////////////////////////
/// Elevator Simulator in C++
/// Copyright (c) 2026, Tammy Cravit <tammy@tammymakesthings.com>
///
/// SPDX-FileCopyrightText: 2026, Tammy Cravit <tammy@tammymakesthings.com>
/// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include <string_view>

namespace elevator {

inline constexpr std::string_view kVersion = "1.1";

inline constexpr int kFloors = 8;     // floors are numbered 1 .. kFloors
inline constexpr int kIdleLimit = 4;  // park after this many cycles with no movement
inline constexpr std::chrono::seconds kCyclePause{1};  // pause at the end of each cycle

}  // namespace elevator
