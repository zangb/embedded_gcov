# Embedded GCOV
Currently running on host machines because it uses malloc and hardcoded values for the memory sizes and the file name.
Malloc can be replaced with hardcoded memory locations that can be read out via tcl.

How to run:
```
./make_results.sh
```
This compiles the program, runs the program to generate the GCDA output and then runs LCOV to generate the test-report.
Use ```clean.sh``` to remove the test and program output as well as the compiled files.

