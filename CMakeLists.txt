# Minimum version recommended for modern CMake
cmake_minimum_required(VERSION 3.15)
project(os LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified externally
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

# Set compiler to Clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

execute_process(
  COMMAND bash ${CMAKE_SOURCE_DIR}/syscalltbl.sh syscall.tbl syscalltbl.lst
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_executable(os
  src/syscall/memory_mapping.cpp
  src/syscall/list_syscalls.cpp
  src/syscall/kill_process_tree.cpp
  src/configuration.cpp
  src/cpu.cpp
  src/internal_memory.cpp
  src/loader.cpp
  src/logger.cpp
  src/memory.cpp
  src/process.cpp
  src/scheduler.cpp
  src/syscall.cpp
  src/timer.cpp
  main.cpp
)

target_include_directories(os PRIVATE include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)