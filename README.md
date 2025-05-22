# Introduction
C++ rewritten (2025) version of the assignment of Operating System (CO2017) course of the Ho Chi Minh City University of Technology (HCMUT) - VNU-HCM.

This is a ported version of the assignment (written in C) with the following changes:
- Use LLVM/Clang and CMake instead of GNU C Compiler (GCC) and GNU Make.
- Written using object-oriented programming (OOP).
- Make use of the Standard Template Library (STL) of C++ for data structures.
- By default use paging on memory, multi-level queue (MLQ) on CPU scheduling.
- Configuration file format has few changes listed below.

# Usage
## Building from source
Configuring build directory and compile the system table:
```shell
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```
Start building:
```shell
cmake --build build
```
## Simulation configuration structure
The example will be shown at `input/` and `output/` of this repository.
```
[simulation time] [# cores] [# processes]
[ram size] [swap 0 size] [swap 1 size] [swap 2 size] [swap 3 size]
[arrival time 0] [path to process 0 from the executable] [priority 0]
[arrival time 1] [path to process 1 from the executable] [priority 1]
...
[arrival time n] [path to process n from the executable] [priority n]
```