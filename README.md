# clitime
A simple CLI timer/stopwatch for Unix-based systems. (Windows support in the future...)
Runs in the terminal. (Space) to Start/Stop, (E) to Exit and (R) to Reset. 

## Installation instructions 
### Linux (tested on Ubuntu 20.04)
1. Clone repository 
2. Enter repo directory and `make`.
### macOS (tested on x86)
We need to ensure that the argp header file and linker is available to the compiler, as macOS does not come with it by default.
1. Install the argp library with `brew install argp-standalone`. 
2. If not already done, add the following lines to `~/.bash_profile`: 
  ```bash 
  export C_INCLUDE_PATH=/usr/local/include
  export LIBRARY_PATH=/usr/local/lib
  ```
3. Enter repo directory and `make`.

## Usage 
1. Run with either the `--stopwatch` or `--timer` options. 
2. For the `--timer` option, enter a time in the format `XhYmZs`. Any combination of (hour/minute/second) can be used, but **must be in that order**.  Eg. 10m20s == 10minutes 20seconds, 1h5m == 1hour5minutes, 30s == 30seconds, 2m == 2minutes, etc.


