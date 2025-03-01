#!/usr/bin/bash
cd "$(dirname "$0")"
mkdir -p output
cmake -S . -B build
cmake --build build
cd build
./coverage
cd ..
python3 gcov.py -p output/gcov_output.bin
mkdir -p results

lcov --branch-coverage --mcdc-coverage --capture --directory . --output-file results/coverage.info
lcov --branch-coverage --mcdc-coverage --remove results/coverage.info -o results/filtered_coverage.info \
            '/usr/include/c++/*' '/doctest/*'
genhtml --mcdc-coverage --branch-coverage results/filtered_coverage.info --output-directory results/html

firefox results/html/index.html
