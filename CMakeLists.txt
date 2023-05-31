cmake_minimum_required(VERSION 3.7)
project(lc32sim VERSION 0.1
                LANGUAGES CXX)

set(SOURCES
    src/display.cpp
    src/elf_file.cpp
    src/instruction.cpp
    src/main.cpp
    src/memory.cpp
    src/sim.cpp
)
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
set(RELEASE_FLAGS
    -O3
    -march=native
    -flto
    -DNDEBUG
)
set(DBGINFO_FLAG -ggdb3)
add_executable(sim ${SOURCES})
target_compile_features(sim PUBLIC cxx_std_23)
target_compile_options(sim PUBLIC "${FLAGS}")
target_compile_options(sim PUBLIC "$<$<CONFIG:DEBUG>:${DBGINFO_FLAGS}>")
target_compile_options(sim PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(sim PUBLIC "$<$<CONFIG:RELEASE_DBGINFO>:${RELEASE_FLAGS}>")
target_compile_options(sim PUBLIC "$<$<CONFIG:RELEASE_DBGINFO>:${DBGINFO_FLAGS}>")