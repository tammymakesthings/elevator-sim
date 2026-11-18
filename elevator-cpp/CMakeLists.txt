# CMake file for C++ Elevator Simulator
# Copyright (c) 2026, Tammy Cravit <tammy@tammymakesthings.com>
#
# SPDX-FileCopyrightText: 2026, Tammy Cravit <tammy@tammymakesthings.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(elevator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

# Simulation logic, shared by the program and the tests.
add_library(elevator_core
  src/Elevator.cpp
  src/Commands.cpp
)
target_include_directories(elevator_core PUBLIC include)

if(MSVC)
  target_compile_options(elevator_core PUBLIC /W4)
else()
  target_compile_options(elevator_core PUBLIC -Wall -Wextra -Wpedantic)
endif()

add_executable(elevator src/main.cpp)
target_link_libraries(elevator PRIVATE elevator_core Threads::Threads)

# Tests: plain C++, no framework needed. Run with `ctest`.
include(CTest)
if(BUILD_TESTING)
  add_executable(elevator_tests tests/elevator_tests.cpp)
  target_link_libraries(elevator_tests PRIVATE elevator_core)
  add_test(NAME elevator_tests COMMAND elevator_tests)
endif()
