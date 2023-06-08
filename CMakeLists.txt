cmake_minimum_required(VERSION 3.7)
project(lc32sim VERSION 0.1
                LANGUAGES CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(SOURCES
    src/display.cpp
    src/elf_file.cpp
    src/instruction.cpp
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
    -march=native
    -flto
    -DNDEBUG
)
# Set for debug and release with debug info configurations
set(DBGINFO_FLAGS
    -ggdb3
)
# Set for debug configuration only
set(DEBUG_FLAGS
    -O0
    -DDEBUG_CHECKS # Enables sanity checks in the looped code that could degrade performance
)
add_executable(sim ${SOURCES})
target_compile_features(sim PUBLIC cxx_std_23)
target_compile_options(sim PUBLIC "${FLAGS}")
target_compile_options(sim PUBLIC "$<$<CONFIG:DEBUG>:${DBGINFO_FLAGS}>")
target_compile_options(sim PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(sim PUBLIC "$<$<CONFIG:RELEASE_DBGINFO>:${RELEASE_FLAGS}>")
target_compile_options(sim PUBLIC "$<$<CONFIG:RELEASE_DBGINFO>:${DBGINFO_FLAGS}>")
target_link_libraries(sim PUBLIC ${SDL2_LIBRARIES})