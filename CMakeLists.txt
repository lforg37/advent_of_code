cmake_minimum_required(VERSION 3.30)
project(AdventOfCode25 VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard to use.
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(add_common_options target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()

function(create_aoc_exec source_stem)
    add_executable(aoc_${source_stem} ${source_stem}.cpp)
    add_common_options(aoc_${source_stem})
endfunction()

add_subdirectory(day_01)
add_subdirectory(day_02)
