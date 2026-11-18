#include "elevator/Elevator.hpp"

#include <algorithm>
#include <cctype>

namespace elevator {

std::string
to_string(Direction d)
{
  switch (d) {
    case Direction::Up:
      return "up";
    case Direction::Down:
      return "down";
    case Direction::Idle:
      break;
  }
  return "idle";
}

namespace {

int
step_of(Direction d)
{
  return static_cast<int>(d);
}

std::string
upper(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  return s;
}

} // namespace

Elevator::Elevator(std::ostream& out,
                   int floors,
                   int idleLimit,
                   std::uint32_t seed)
  : out_(out)
  , floors_(floors)
  , idleLimit_(idleLimit)
  , rng_(seed)
  , up_(static_cast<std::size_t>(floors), false)
  , down_(static_cast<std::size_t>(floors), false)
  , car_(static_cast<std::size_t>(floors), false)
{
}

std::ostream&
Elevator::log() const
{
  return out_ << '[' << cycle_ << "] ";
}

// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------

void
Elevator::pushUp(int floor)
{
  if (validFloor(floor) && floor < floors_) {
    log() << "UP pressed on floor " << floor << '\n';
    up_[index(floor)] = true;
  } else {
    log() << "Ignored: no UP button on floor " << floor << '\n';
  }
}

void
Elevator::pushDown(int floor)
{
  if (validFloor(floor) && floor > 1) {
    log() << "DOWN pressed on floor " << floor << '\n';
    down_[index(floor)] = true;
  } else {
    log() << "Ignored: no DOWN button on floor " << floor << '\n';
  }
}

void
Elevator::pushCar(int floor)
{
  if (validFloor(floor)) {
    log() << "Car button pressed for floor " << floor << '\n';
    car_[index(floor)] = true;
  } else {
    log() << "Ignored: no floor " << floor << '\n';
  }
}

// Press one random button: a random floor, then UP, DOWN or car. UP on the
// top floor becomes DOWN, and DOWN on the bottom floor becomes UP, since
// those buttons don't exist.
void
Elevator::pressOneRandom()
{
  std::uniform_int_distribution<int> floorDist(1, floors_);
  std::uniform_int_distribution<int> kindDist(0, 2);
  const int floor = floorDist(rng_);
  switch (kindDist(rng_)) {
    case 0:
      if (floor == floors_)
        pushDown(floor);
      else
        pushUp(floor);
      break;
    case 1:
      if (floor == 1)
        pushUp(floor);
      else
        pushDown(floor);
      break;
    default:
      pushCar(floor);
      break;
  }
}

bool
Elevator::pressRandom(int count)
{
  if (count < 1) {
    log() << "Ignored: r needs a count of 1 or more\n";
    return false;
  }
  if (count > floors_ * 3) {
    log() << "Ignored: r count can't be more than " << floors_ * 3 << '\n';
    return false;
  }
  log() << "Pressing " << count << " random button(s)\n";
  for (int i = 0; i < count; ++i)
    pressOneRandom();
  return true;
}

// ---------------------------------------------------------------------------
// Priority calls
// ---------------------------------------------------------------------------

void
Elevator::setPriority(int floor)
{
  if (!validFloor(floor)) {
    log() << "Ignored: no floor " << floor << '\n';
    return;
  }
  log() << "PRIORITY call to floor " << floor
        << " -- normal service suspended\n";
  priority_ = floor;
  holding_ = false;
}

void
Elevator::clearPriority()
{
  if (!priority_) {
    log() << "No priority call is active\n";
    return;
  }
  log() << "PRIORITY cleared -- resuming normal service\n";
  priority_.reset();
  holding_ = false;
  direction_ = Direction::Idle;
  idleCycles_ = 0;
  doorAge_ = 0; // if doors are open, give one boarding cycle, then close
}

// ---------------------------------------------------------------------------
// Looking at the state
// ---------------------------------------------------------------------------

bool
Elevator::requestAt(int floor) const
{
  const auto i = index(floor);
  return up_[i] || down_[i] || car_[i];
}

bool
Elevator::anyRequests(int lo, int hi) const
{
  for (int f = lo; f <= hi; ++f) {
    if (requestAt(f))
      return true;
  }
  return false;
}

bool
Elevator::requestsAbove() const
{
  return anyRequests(currentFloor_ + 1, floors_);
}
bool
Elevator::requestsBelow() const
{
  return anyRequests(1, currentFloor_ - 1);
}

// Should we stop at the current floor?
// - Always stop if someone in the car asked for this floor.
// - Going up: stop for an UP call, or for a DOWN call if nothing is above us
//   (we're about to turn around). Going down is the mirror image.
// - Idle: stop for any call here.
bool
Elevator::shouldStop() const
{
  const auto i = index(currentFloor_);
  if (car_[i])
    return true;
  switch (direction_) {
    case Direction::Up:
      return up_[i] || (down_[i] && !requestsAbove());
    case Direction::Down:
      return down_[i] || (up_[i] && !requestsBelow());
    case Direction::Idle:
      break;
  }
  return requestAt(currentFloor_);
}

// Pick a direction, preferring to keep going the way we're already going.
Direction
Elevator::chooseDirection() const
{
  if (direction_ == Direction::Up && requestsAbove())
    return Direction::Up;
  if (direction_ == Direction::Down && requestsBelow())
    return Direction::Down;
  if (requestsAbove())
    return Direction::Up;
  if (requestsBelow())
    return Direction::Down;
  return Direction::Idle;
}

int
Elevator::nextFloor()
{
  if (shouldStop())
    return currentFloor_;
  direction_ = chooseDirection();
  return currentFloor_ + step_of(direction_);
}

// ---------------------------------------------------------------------------
// Doors
// ---------------------------------------------------------------------------

// Clear the call(s) answered at this floor. The car button is always
// cleared. For hall buttons: if we'll keep going the same way, only clear
// that direction's button; otherwise clear both. Also run while the doors
// are open, so a button pressed for this floor during boarding is answered
// without reopening.
void
Elevator::clearCalls()
{
  const auto i = index(currentFloor_);
  car_[i] = false;
  if (direction_ == Direction::Up && requestsAbove()) {
    up_[i] = false;
  } else if (direction_ == Direction::Down && requestsBelow()) {
    down_[i] = false;
  } else {
    up_[i] = false;
    down_[i] = false;
  }
}

void
Elevator::openDoors()
{
  doorsOpen_ = true;
  doorAge_ = 0;
  log() << "Doors open at floor " << currentFloor_ << '\n';
}

void
Elevator::closeDoors()
{
  doorsOpen_ = false;
  log() << "Doors closing at floor " << currentFloor_ << '\n';
}

// Called each cycle while the doors are open: the cycle after opening is
// for boarding, the next one closes them.
void
Elevator::doorStep()
{
  clearCalls();
  if (++doorAge_ == 1) {
    log() << "Passengers boarding at floor " << currentFloor_ << '\n';
  } else {
    closeDoors();
  }
}

// ---------------------------------------------------------------------------
// Simulation
// ---------------------------------------------------------------------------

bool
Elevator::step()
{
  if (doorsOpen_) {
    doorStep();
    return true;
  }
  if (shouldStop()) {
    clearCalls();
    openDoors();
    return true;
  }
  const int next = nextFloor();
  if (next == currentFloor_)
    return false;
  currentFloor_ = next;
  log() << "Moving " << to_string(direction_) << " to floor " << currentFloor_
        << '\n';
  return true;
}

// Priority mode: head straight for the priority floor, ignoring every other
// request. Doors open somewhere else close at once (no boarding cycle); at
// the priority floor they open and stay open until the priority is cleared.
void
Elevator::priorityStep()
{
  const int distance = *priority_ - currentFloor_;
  if (distance == 0) {
    if (!holding_) {
      holding_ = true;
      direction_ = Direction::Idle;
      if (!doorsOpen_)
        openDoors();
      log() << "PRIORITY: holding at floor " << currentFloor_
            << " with doors open\n";
    }
  } else if (doorsOpen_) {
    closeDoors();
  } else {
    direction_ = distance > 0 ? Direction::Up : Direction::Down;
    currentFloor_ += step_of(direction_);
    log() << "PRIORITY: moving " << to_string(direction_) << " to floor "
          << currentFloor_ << '\n';
  }
}

// Normal-mode bookkeeping: park after idleLimit cycles without movement.
void
Elevator::countIdle(bool active)
{
  if (active) {
    idleCycles_ = 0;
    return;
  }
  if (++idleCycles_ == idleLimit_) {
    direction_ = Direction::Idle;
    log() << "No movement for " << idleLimit_ << " cycles -- parked at floor "
          << currentFloor_ << '\n';
  }
}

void
Elevator::tick()
{
  if (priority_) {
    priorityStep();
  } else {
    countIdle(step());
  }
  ++cycle_;
}

// ---------------------------------------------------------------------------
// State display
// ---------------------------------------------------------------------------

std::string
Elevator::pressedList(const std::vector<bool>& buttons) const
{
  std::string result;
  for (int f = 1; f <= floors_; ++f) {
    if (!buttons[index(f)])
      continue;
    if (!result.empty())
      result += ", ";
    result += std::to_string(f);
  }
  return result.empty() ? "none" : result;
}

void
Elevator::printState() const
{
  out_ << "-----\n"
       << "Current state: " << upper(to_string(direction_)) << " on floor "
       << currentFloor_ << '\n'
       << "Doors: " << (doorsOpen_ ? "OPEN" : "CLOSED") << '\n'
       << "Priority: ";
  if (priority_) {
    out_ << "floor " << *priority_ << (holding_ ? " (holding)" : " (en route)");
  } else {
    out_ << "none";
  }
  out_ << '\n'
       << "Up buttons: " << pressedList(up_) << '\n'
       << "Down buttons: " << pressedList(down_) << '\n'
       << "Car buttons: " << pressedList(car_) << '\n'
       << "-----\n";
}

} // namespace elevator
