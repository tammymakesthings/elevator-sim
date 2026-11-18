// Elevator simulation -- interactive console front end.
//
// A background thread reads lines from stdin and queues them; the main
// thread runs the simulation, handling everything typed since the last
// cycle at the start of each cycle, so the elevator never waits on the
// keyboard.

#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <thread>

#include "elevator/Commands.hpp"
#include "elevator/Config.hpp"
#include "elevator/Elevator.hpp"

namespace {

// Lines typed by the user. std::nullopt marks end of input.
class LineQueue
{
public:
  void push(std::optional<std::string> line)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    lines_.push_back(std::move(line));
  }

  // Returns false if the queue is empty.
  bool pop(std::optional<std::string>& line)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (lines_.empty())
      return false;
    line = std::move(lines_.front());
    lines_.pop_front();
    return true;
  }

private:
  std::mutex mutex_;
  std::deque<std::optional<std::string>> lines_;
};

} // namespace

int
main()
{
  using namespace elevator;

  // Shared with the reader thread, which may outlive main's locals.
  auto queue = std::make_shared<LineQueue>();

  // There's no portable way to interrupt a blocked std::getline, so the
  // reader is detached; it ends with the process when we return.
  std::thread([queue] {
    std::string line;
    while (std::getline(std::cin, line))
      queue->push(line);
    queue->push(std::nullopt);
  }).detach();

  Elevator elevator(std::cout, kFloors, kIdleLimit, std::random_device{}());
  printHelp(std::cout, kFloors);

  bool running = true;
  while (running) {
    std::optional<std::string> line;
    while (running && queue->pop(line)) {
      running =
        handleCommand(elevator, std::cout, line) == CommandResult::Continue;
    }
    if (!running)
      break;

    elevator.tick();
    std::cout.flush();
    std::this_thread::sleep_for(kCyclePause);
  }

  std::cout << "Stopped at floor " << elevator.currentFloor() << std::endl;
  return 0;
}
