#pragma once

#include <cstdint>
#include <optional>
#include <ostream>
#include <random>
#include <string>
#include <vector>

namespace elevator {

enum class Direction { Down = -1, Idle = 0, Up = 1 };

// Lowercase name ("up", "down", "idle").
std::string to_string(Direction d);

// A single elevator serving floors 1..floors.
//
// It uses a "LOOK" strategy: it keeps moving in its current direction while
// there are requests ahead of it, and only reverses when there is nothing
// left in that direction. Each call to tick() is one simulation cycle.
//
// All output goes to the stream passed to the constructor, each line
// prefixed with the cycle number, e.g. "[12] Moving up to floor 3".
class Elevator {
public:
    Elevator(std::ostream& out, int floors, int idleLimit, std::uint32_t seed);

    // --- Buttons --------------------------------------------------------
    void pushUp(int floor);    // no UP button on the top floor
    void pushDown(int floor);  // no DOWN button on the bottom floor
    void pushCar(int floor);   // button inside the car

    // Press `count` random buttons. Returns false (and presses nothing) if
    // count is outside 1 .. floors*3.
    bool pressRandom(int count);

    // --- Priority (e.g. fire service) -----------------------------------
    // Go straight to `floor`, ignoring all other requests, and hold there
    // with the doors open until clearPriority() is called.
    void setPriority(int floor);
    void clearPriority();

    // --- Simulation -----------------------------------------------------
    void tick();  // run one cycle
    void printState() const;

    // Start a log line: writes the "[cycle] " prefix and returns the stream.
    std::ostream& log() const;

    // --- Observers (used by the tests) ----------------------------------
    int floors() const { return floors_; }
    int currentFloor() const { return currentFloor_; }
    int cycle() const { return cycle_; }
    Direction direction() const { return direction_; }
    bool doorsOpen() const { return doorsOpen_; }
    std::optional<int> priority() const { return priority_; }
    bool holding() const { return holding_; }
    bool upPressed(int floor) const { return up_[index(floor)]; }
    bool downPressed(int floor) const { return down_[index(floor)]; }
    bool carPressed(int floor) const { return car_[index(floor)]; }

private:
    // Button arrays are indexed 0 .. floors-1 for floors 1 .. floors.
    static std::size_t index(int floor) { return static_cast<std::size_t>(floor - 1); }

    bool validFloor(int floor) const { return floor >= 1 && floor <= floors_; }
    bool requestAt(int floor) const;
    bool anyRequests(int lo, int hi) const;  // any button for floors lo..hi
    bool requestsAbove() const;
    bool requestsBelow() const;

    bool shouldStop() const;
    Direction chooseDirection() const;
    int nextFloor();  // also records the chosen direction

    void clearCalls();
    void openDoors();
    void closeDoors();
    void doorStep();

    bool step();  // normal service; returns true if anything happened
    void priorityStep();
    void countIdle(bool active);
    void pressOneRandom();

    std::string pressedList(const std::vector<bool>& buttons) const;

    std::ostream& out_;
    int floors_;
    int idleLimit_;
    std::mt19937 rng_;

    std::vector<bool> up_;
    std::vector<bool> down_;
    std::vector<bool> car_;

    int currentFloor_ = 1;
    Direction direction_ = Direction::Idle;
    int idleCycles_ = 0;
    int cycle_ = 0;

    bool doorsOpen_ = false;
    int doorAge_ = 0;  // cycles since the doors opened

    std::optional<int> priority_;
    bool holding_ = false;  // true once we've arrived at the priority floor
};

}  // namespace elevator
