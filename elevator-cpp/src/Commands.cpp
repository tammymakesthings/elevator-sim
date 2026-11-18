//////////////////////////////////////////////////////////////////////////////
/// Elevator Simulator in C++
/// Copyright (c) 2026, Tammy Cravit <tammy@tammymakesthings.com>
///
/// SPDX-FileCopyrightText: 2026, Tammy Cravit <tammy@tammymakesthings.com>
/// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "elevator/Commands.hpp"

#include <charconv>
#include <sstream>
#include <vector>

#include "elevator/Config.hpp"

namespace elevator {

namespace {

std::vector<std::string>
split(const std::string& line)
{
  std::istringstream in(line);
  std::vector<std::string> words;
  for (std::string w; in >> w;)
    words.push_back(w);
  return words;
}

std::optional<int>
parseInt(const std::string& s)
{
  int value = 0;
  const char* first = s.data();
  const char* last = s.data() + s.size();
  const auto [ptr, ec] = std::from_chars(first, last, value);
  if (ec != std::errc{} || ptr != last)
    return std::nullopt;
  return value;
}

void
badCommand(const Elevator& elevator)
{
  elevator.log() << "Unknown command -- type h for help\n";
}

} // namespace

void
printBanner(std::ostream& out, int floors)
{
  out << "Elevator simulation v" << kVersion << " -- " << floors << " floors\n";
}

void
printHelp(std::ostream& out, int floors)
{
  printBanner(out, floors);
  out << "Commands (floors are 1-" << floors << "):\n"
      << "  u N   press UP on floor N\n"
      << "  d N   press DOWN on floor N\n"
      << "  c N   press floor N inside the car\n"
      << "  r N   press N random buttons (N = 1-" << floors * 3 << ")\n"
      << "  p N   priority call: go straight to floor N and hold there\n"
      << "  x     clear the priority call\n"
      << "  s     show current state\n"
      << "  h, ?  show this help\n"
      << "  v     show version\n"
      << "  q     quit\n";
}

void
printVersion(std::ostream& out)
{
  out << "Elevator simulation version " << kVersion << '\n';
}

CommandResult
handleCommand(Elevator& elevator,
              std::ostream& out,
              const std::optional<std::string>& line)
{
  if (!line)
    return CommandResult::Quit; // end of input

  const auto words = split(*line);
  if (words.empty())
    return CommandResult::Continue; // blank line
  const std::string& cmd = words[0];

  if (words.size() == 1) {
    if (cmd == "q")
      return CommandResult::Quit;
    if (cmd == "s")
      elevator.printState();
    else if (cmd == "x")
      elevator.clearPriority();
    else if (cmd == "h" || cmd == "?")
      printHelp(out, elevator.floors());
    else if (cmd == "v")
      printVersion(out);
    else
      badCommand(elevator);
    return CommandResult::Continue;
  }

  const auto n = words.size() == 2 ? parseInt(words[1]) : std::nullopt;
  if (!n) {
    badCommand(elevator);
    return CommandResult::Continue;
  }

  if (cmd == "u")
    elevator.pushUp(*n);
  else if (cmd == "d")
    elevator.pushDown(*n);
  else if (cmd == "c")
    elevator.pushCar(*n);
  else if (cmd == "r") {
    if (elevator.pressRandom(*n))
      elevator.printState();
  } else if (cmd == "p")
    elevator.setPriority(*n);
  else
    badCommand(elevator);
  return CommandResult::Continue;
}

} // namespace elevator
