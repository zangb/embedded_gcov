# Embedded GCOV
Currently running on host machines because it uses malloc and hardcoded values for the memory sizes.

How to run:
```
mkdir output
./compile.sh
cd build    // important because of hardcoded output path
./coverage
cd ..
./make_results.sh
```

