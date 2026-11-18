#!/usr/bin/env bash

cmake -S . -B build
cmake --build build
# ctest --test-dir build    # run the tests

