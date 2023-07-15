cmake_minimum_required(VERSION 3.14)
project(lc32sim VERSION 0.1
                LANGUAGES CXX)

# SDL2 is required for the display
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Boost is used for config file parsing and general utilities
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.78.0 REQUIRED COMPONENTS )
include_directories(${Boost_INCLUDE_DIRS})

# Argparse is used for command line argument parsing
# This might be installed globally, in the case of a flake build. Therefore, add
# an option for that, which is off by default.
option(LC32SIM_SYSTEM_ARGPARSE "Use the version of `argparse` installed on the system" OFF)
if(NOT LC32SIM_SYSTEM_ARGPARSE)
    # We have to use a custom setup for FetchContent_MakeAvailable because we
    # don't want to install these dependencies.
    # See: https://cmake.org/cmake/help/v3.19/module/FetchContent.html
    include(FetchContent)
    FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
        GIT_TAG 997da9255618311d1fcb0135ce86022729d1f1cb
    )
    FetchContent_GetProperties(argparse)
    if(NOT argparse_POPULATED)
        FetchContent_Populate(argparse)
        add_subdirectory(${argparse_SOURCE_DIR} ${argparse_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
else()
    find_package(argparse 2.9.0 REQUIRED)
endif()
# Need to populate variables for argparse
# Include directories are handled automatically, either by linking with the
# library or because they are found by the compiler. We only need to link with
# the target if we're using our own.
if(NOT LC32SIM_SYSTEM_ARGPARSE)
    set(argparse_LIBRARIES "argparse")
endif()

set(SOURCES
    src/config.cpp
    src/display.cpp
    src/elf_file.cpp
    src/instruction.cpp
    src/log.cpp
    src/main.cpp
    src/memory.cpp
    src/sim.cpp
)
# Set for all configurations
set(FLAGS
    -Wextra
    -Wall
    -Wno-deprecated
    -Wno-unknown-warning-option
    -Wno-self-assign
    -Wno-unused-parameter
    -Wno-unused-but-set-variable
    -Wold-style-cast
    -Waddress-of-packed-member
)
# Set for release and release with debug info configurations
set(RELEASE_FLAGS
    -O3
    -flto
    -funroll-loops
    -DNDEBUG
)
# Set for debug and release with debug info configurations
set(DBGINFO_FLAGS
    -ggdb3
)
# Set for debug configuration only
set(DEBUG_FLAGS
    -Og
)
add_executable(lc32sim ${SOURCES})
target_compile_features(lc32sim PRIVATE cxx_std_23)
target_compile_options(lc32sim PRIVATE "${FLAGS}")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:DEBUG>:${DBGINFO_FLAGS}>")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:RELEASE_DBGINFO>:${RELEASE_FLAGS}>")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:RELEASE_DBGINFO>:${DBGINFO_FLAGS}>")
target_link_libraries(lc32sim PRIVATE ${SDL2_LIBRARIES} ${Boost_LIBRARIES} ${argparse_LIBRARIES})

install(TARGETS lc32sim)
