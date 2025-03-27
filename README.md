# biam
Biologically-inspired algorithms and models

## Usage
To build all the executables run
```
make
```
Executables should appear in `build/` directory

## test_qap
This is program to benchmark different qap methods, it prints user friendly output to stderr
but writes results in csv format to stdout, if you want to put it into a file just redirect stream
```
./build/test_qap ~/datasets/qaplib/lipa20a.dat -K 10 > results.csv
```
There are two parameters:
* `-K` - number of iterations
* `-l` - just benchmark the local search method (greedy and steepest)
