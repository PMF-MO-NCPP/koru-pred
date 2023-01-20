cmake_minimum_required(VERSION 3.13)
project(koru-pred LANGUAGES CXX)

# Za rad u praktikumu
set(CMAKE_CXX_COMPILER /usr/bin/g++-12  CACHE FILEPATH "Staza do c++ prevoditelja")

# Izbjegava bug u instalaciji VS Code okoline u praktikumu.
set(CMAKE_BUILD_TYPE Release CACHE STRING "Manually set build type")

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG   9.1.0
)

FetchContent_MakeAvailable(fmt)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)

add_executable(simple simple.cpp)
target_link_libraries(simple PRIVATE fmt::fmt-header-only)

add_executable(gen generator.cpp)
target_link_libraries(gen PRIVATE fmt::fmt-header-only)

add_executable(gen-it generator-it.cpp)
target_link_libraries(gen-it PRIVATE fmt::fmt-header-only)

add_executable(yield_back yield_back.cpp)
target_link_libraries(yield_back PRIVATE fmt::fmt-header-only)

add_executable(coro-coro coro-coro.cpp)
target_link_libraries(coro-coro PRIVATE fmt::fmt-header-only)
