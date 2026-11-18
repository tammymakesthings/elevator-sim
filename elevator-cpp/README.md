# Elevator simulation (C++17)

A C++17 port of the 8th elevator simulation, version 1.0. It has the same
behavior and the same commands.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/elevator
```

Run the tests with:

```sh
ctest --test-dir build --output-on-failure
```

Requirements: a C++17 compiler and CMake 3.16 or later. It uses only the
standard library (plus the platform's thread library, found through CMake's
`Threads` package).

## Commands

| Command | Action |
|---------|--------|
| `u N`   | press UP on floor N |
| `d N`   | press DOWN on floor N |
| `c N`   | press floor N inside the car |
| `r N`   | press N random buttons (1 to floors*3), then show the state |
| `p N`   | priority call: go straight to floor N and hold there |
| `x`     | clear the priority call |
| `s`     | show the current state |
| `h`, `?`| show help |
| `v`     | show the version |
| `q`     | quit (Ctrl-D also quits) |

## Layout

| File | Contents |
|------|----------|
| `include/elevator/Config.hpp`   | version, floor count, idle limit, cycle pause |
| `include/elevator/Elevator.hpp`, `src/Elevator.cpp` | the `Elevator` class: buttons, LOOK scheduling, doors, priority, parking |
| `include/elevator/Commands.hpp`, `src/Commands.cpp` | command parsing, help, version |
| `src/main.cpp`   | console front end: keyboard-reader thread and main loop |
| `tests/elevator_tests.cpp` | tests, with no framework needed |

## Behavior

- Floors are numbered 1 to `kFloors` (8 by default).
- The elevator keeps moving in its current direction while there are
  requests ahead of it, and only reverses when there are none left.
- At each stop it opens the doors, spends one cycle boarding, closes the
  doors, then moves on. A button pressed for the current floor while the
  doors are open is answered without reopening them.
- After 4 cycles with no movement it parks where it is.
- A priority call goes straight to its floor without stopping and holds
  there with the doors open. If the doors were open somewhere else, they
  close at once. Buttons pressed during a priority call are kept and
  served once the priority is cleared.
- Each cycle starts by handling everything typed since the last cycle, and
  ends with a 1-second pause.

The `Elevator` class writes to any `std::ostream` and takes a random seed, so
the tests can run it deterministically without a terminal.
