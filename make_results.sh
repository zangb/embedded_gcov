python3 gcov.py -p output/gcov_output.bin
mkdir -p results

lcov --branch-coverage --mcdc-coverage --capture --directory . --output-file results/coverage.info
lcov --remove results/coverage.info -o results/filtered_coverage.info \
            '/usr/include/c++/*' '/doctest/*'
genhtml --branch-coverage results/filtered_coverage.info --output-directory results/html

firefox results/html/index.html
