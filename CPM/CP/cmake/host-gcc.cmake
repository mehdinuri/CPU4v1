# Host GCC toolchain for x86-64 unit testing
# Used with the Host-Test CMake preset.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

find_program(CMAKE_C_COMPILER   NAMES gcc   cc  REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES g++   c++ REQUIRED)

# Coverage + sanitizers for Debug host builds
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3 -fprofile-arcs -ftest-coverage -fsanitize=address,undefined")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG
    "-fprofile-arcs -ftest-coverage -fsanitize=address,undefined")

# Suppress cross-compile checks (we're building natively)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
