project(find-substr-simd)
cmake_minimum_required(VERSION 3.19)

add_executable(find-substr-simd src/main.cpp)

add_subdirectory(third_party/googletest)

set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(third_party/benchmark)

target_link_libraries(find-substr-simd PRIVATE gtest benchmark::benchmark)

target_compile_options(find-substr-simd PRIVATE -march=native -mavx -mavx2)
