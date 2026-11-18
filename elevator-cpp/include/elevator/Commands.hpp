#pragma once

#include <optional>
#include <ostream>
#include <string>

#include "elevator/Elevator.hpp"

namespace elevator {

enum class CommandResult { Continue, Quit };

void printBanner(std::ostream& out, int floors);
void printHelp(std::ostream& out, int floors);
void printVersion(std::ostream& out);

// Handle one line typed by the user. std::nullopt means end of input
// (Ctrl-D), which quits just like "q".
//
// Commands:
//   u N   press UP on floor N
//   d N   press DOWN on floor N
//   c N   press floor N inside the car
//   r N   press N random buttons, then show the state
//   p N   priority call: go straight to floor N and hold there
//   x     clear the priority call
//   s     show current state
//   h, ?  show help
//   v     show version
//   q     quit
CommandResult handleCommand(Elevator& elevator, std::ostream& out,
                            const std::optional<std::string>& line);

}  // namespace elevator
