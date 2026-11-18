//////////////////////////////////////////////////////////////////////////////
/// Elevator Simulator in C++
/// Copyright (c) 2026, Tammy Cravit <tammy@tammymakesthings.com>
///
/// SPDX-FileCopyrightText: 2026, Tammy Cravit <tammy@tammymakesthings.com>
/// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

// Tests for the elevator simulation. No framework: each test is a function,
// CHECK records failures, and main() returns non-zero if anything failed.

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "elevator/Config.hpp"
#include "elevator/Commands.hpp"
#include "elevator/Elevator.hpp"

using namespace elevator;

namespace {

int failures = 0;

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      ++failures;                                                              \
      std::cerr << "  FAILED: " #cond " (" << __FILE__ << ':' << __LINE__      \
                << ")\n";                                                      \
    }                                                                          \
  } while (0)

bool
contains(const std::string& haystack, const std::string& needle)
{
  return haystack.find(needle) != std::string::npos;
}

struct Fixture
{
  std::ostringstream out;
  Elevator e{ out, 8, 4, 12345 };

  // Tick until the doors open, up to `limit` cycles. Returns the floor,
  // or -1 if they never opened.
  int runToNextStop(int limit = 30)
  {
    for (int i = 0; i < limit; ++i) {
      const bool wasOpen = e.doorsOpen();
      e.tick();
      if (!wasOpen && e.doorsOpen())
        return e.currentFloor();
    }
    return -1;
  }

  // Floors where the doors opened, in order, over `cycles` ticks.
  std::vector<int> stopsOver(int cycles)
  {
    std::vector<int> stops;
    for (int i = 0; i < cycles; ++i) {
      const bool wasOpen = e.doorsOpen();
      e.tick();
      if (!wasOpen && e.doorsOpen())
        stops.push_back(e.currentFloor());
    }
    return stops;
  }
};

// ---------------------------------------------------------------------------

void
testInvalidButtonsIgnored()
{
  Fixture f;
  f.e.pushUp(8);   // no UP on the top floor
  f.e.pushDown(1); // no DOWN on the bottom floor
  f.e.pushCar(0);
  f.e.pushCar(9);
  for (int floor = 1; floor <= 8; ++floor) {
    CHECK(!f.e.upPressed(floor));
    CHECK(!f.e.downPressed(floor));
    CHECK(!f.e.carPressed(floor));
  }
  CHECK(contains(f.out.str(), "Ignored: no UP button on floor 8"));
  CHECK(contains(f.out.str(), "Ignored: no DOWN button on floor 1"));
}

void
testKeepsDirectionBeforeReversing()
{
  // Going up, the elevator passes a DOWN call on 3 (there are requests
  // above), serves 5, then comes back down for 3.
  Fixture f;
  f.e.pushDown(3);
  f.e.pushUp(5);
  CHECK((f.stopsOver(30) == std::vector<int>{ 5, 3 }));
}

void
testStopsForDownCallAtTopOfTrip()
{
  // Nothing above 6, so the DOWN call there is where it turns around.
  Fixture f;
  f.e.pushDown(6);
  f.e.pushCar(2);
  CHECK((f.stopsOver(30) == std::vector<int>{ 2, 6 }));
}

void
testDoorCycle()
{
  // Arrive, open, board for one cycle, close, then move.
  Fixture f;
  f.e.pushCar(2);
  f.e.pushCar(3);
  f.e.tick(); // move to 2
  CHECK(f.e.currentFloor() == 2 && !f.e.doorsOpen());
  f.e.tick(); // open
  CHECK(f.e.doorsOpen());
  f.e.tick(); // boarding
  CHECK(f.e.doorsOpen() && f.e.currentFloor() == 2);
  f.e.tick(); // close
  CHECK(!f.e.doorsOpen() && f.e.currentFloor() == 2);
  f.e.tick(); // move to 3
  CHECK(f.e.currentFloor() == 3);
  CHECK(contains(f.out.str(), "Passengers boarding at floor 2"));
}

void
testButtonForThisFloorDuringBoardingDoesNotReopen()
{
  Fixture f;
  f.e.pushCar(2);
  CHECK(f.runToNextStop() == 2);
  f.e.pushCar(2); // pressed while the doors are open
  f.e.tick();     // boarding: answers it
  CHECK(!f.e.carPressed(2));
  f.e.tick(); // close
  CHECK(f.stopsOver(10).empty());
}

void
testParksAfterIdleLimit()
{
  Fixture f;
  for (int i = 0; i < 4; ++i)
    f.e.tick();
  CHECK(contains(f.out.str(), "No movement for 4 cycles -- parked at floor 1"));
  CHECK(f.e.direction() == Direction::Idle);
}

void
testPriorityOverridesAndHolds()
{
  Fixture f;
  f.e.pushCar(6);
  f.runToNextStop(); // at 6, doors open
  f.e.pushCar(3);
  f.e.pushUp(7);
  f.e.setPriority(1);

  f.e.tick(); // doors close at once, no boarding cycle
  CHECK(!f.e.doorsOpen());
  for (int i = 0; i < 5; ++i)
    f.e.tick(); // 6 -> 1, no stops on the way
  CHECK(f.e.currentFloor() == 1 && !f.e.doorsOpen());
  f.e.tick(); // arrive: open and hold
  CHECK(f.e.holding() && f.e.doorsOpen());
  for (int i = 0; i < 10; ++i)
    f.e.tick(); // stays put, doors open
  CHECK(f.e.currentFloor() == 1 && f.e.doorsOpen());
  CHECK(f.e.carPressed(3) && f.e.upPressed(7)); // still waiting

  f.e.clearPriority();
  CHECK(!f.e.priority());
  CHECK((f.stopsOver(30) == std::vector<int>{ 3, 7 }));
}

void
testRandomLimits()
{
  Fixture f;
  CHECK(!f.e.pressRandom(0));
  CHECK(!f.e.pressRandom(25)); // 8 floors * 3
  CHECK(contains(f.out.str(), "r count can't be more than 24"));
  CHECK(f.e.pressRandom(24));
  CHECK(!contains(f.out.str(), "Ignored: no UP"));   // never presses a
  CHECK(!contains(f.out.str(), "Ignored: no DOWN")); // missing button
}

void
testCommands()
{
  Fixture f;
  CHECK(handleCommand(f.e, f.out, std::string("u 3")) ==
        CommandResult::Continue);
  CHECK(f.e.upPressed(3));
  handleCommand(f.e, f.out, std::string("  c   5  "));
  CHECK(f.e.carPressed(5));
  handleCommand(f.e, f.out, std::string("u x"));
  handleCommand(f.e, f.out, std::string("z"));
  CHECK(contains(f.out.str(), "Unknown command -- type h for help"));
  CHECK(handleCommand(f.e, f.out, std::string("")) == CommandResult::Continue);
  handleCommand(f.e, f.out, std::string("v"));
  CHECK(contains(f.out.str(), "Elevator simulation version"));
  handleCommand(f.e, f.out, std::string("?"));
  CHECK(contains(f.out.str(), "\nCommands"));
  handleCommand(f.e, f.out, std::string("r 2"));
  CHECK(contains(f.out.str(), "Current state: ")); // state shown after r
  CHECK(handleCommand(f.e, f.out, std::string("q")) == CommandResult::Quit);
  CHECK(handleCommand(f.e, f.out, std::nullopt) == CommandResult::Quit);
}

void
testStateDisplay()
{
  Fixture f;
  f.e.pushUp(1);
  f.e.pushUp(5);
  f.e.pushDown(5);
  f.e.pushCar(2);
  f.out.str("");
  f.e.printState();
  CHECK(f.out.str() == "-----\n"
                       "Current state: IDLE on floor 1\n"
                       "Doors: CLOSED\n"
                       "Priority: none\n"
                       "Up buttons: 1, 5\n"
                       "Down buttons: 5\n"
                       "Car buttons: 2\n"
                       "-----\n");
}

} // namespace

int
main()
{
  const std::vector<std::pair<const char*, std::function<void()>>> tests = {
    { "invalid buttons are ignored", testInvalidButtonsIgnored },
    { "keeps direction before reversing", testKeepsDirectionBeforeReversing },
    { "stops for DOWN call at top of trip", testStopsForDownCallAtTopOfTrip },
    { "door open / board / close cycle", testDoorCycle },
    { "button during boarding doesn't reopen",
      testButtonForThisFloorDuringBoardingDoesNotReopen },
    { "parks after idle limit", testParksAfterIdleLimit },
    { "priority overrides and holds", testPriorityOverridesAndHolds },
    { "random press limits", testRandomLimits },
    { "command parsing", testCommands },
    { "state display format", testStateDisplay },
  };

  for (const auto& [name, test] : tests) {
    const int before = failures;
    test();
    std::cout << (failures == before ? "PASS  " : "FAIL  ") << name << '\n';
  }
  std::cout << (failures == 0 ? "All tests passed\n" : "Some tests FAILED\n");
  return failures == 0 ? 0 : 1;
}
