cmake_minimum_required(VERSION 3.11)
project(lc32sim VERSION 0.1
                LANGUAGES CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(SOURCES
    src/config.cpp
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
add_executable(lc32sim ${SOURCES})
target_compile_features(lc32sim PRIVATE cxx_std_23)
target_compile_options(lc32sim PRIVATE "${FLAGS}")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:DEBUG>:${DBGINFO_FLAGS}>")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:RELEASE_DBGINFO>:${RELEASE_FLAGS}>")
target_compile_options(lc32sim PRIVATE "$<$<CONFIG:RELEASE_DBGINFO>:${DBGINFO_FLAGS}>")
target_link_libraries(lc32sim PRIVATE ${SDL2_LIBRARIES})


if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(lc32sim PRIVATE nlohmann_json::nlohmann_json)