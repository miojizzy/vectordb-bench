# Copyright 2026 VectorDBBench Authors
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.16)
project(hnswlib_benchmark VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# Enable AVX for arena-hnswlib SIMD optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -mavx)
  # Disable specific warnings for third-party code
  add_compile_options(-Wno-pedantic -Wno-ignored-qualifiers)
  # Fix GCC 13 + benchmark compatibility issue
  add_compile_options(-fno-tree-vectorize)
endif()

# =============================================================================
# Dependencies
# =============================================================================

include(FetchContent)

# Google Benchmark
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

# Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

# =============================================================================
# Libraries
# =============================================================================

# hnswlib (header-only)
add_library(hnswlib INTERFACE)
target_include_directories(hnswlib INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/hnswlib
)

# arena-hnswlib (header-only)
add_library(arena_hnswlib INTERFACE)
target_include_directories(arena_hnswlib INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/arena-hnswlib/include
)

# Common source files (shared by benchmark and tests)
set(COMMON_SOURCES
  src/dataset_loader.cpp
  src/cli_parser.cpp
  src/index_wrapper.cpp
  src/hnswlib_wrapper.cpp
  src/arena_hnswlib_wrapper.cpp
  src/index_manager.cpp
)

# =============================================================================
# Executables
# =============================================================================

# Benchmark executable
add_executable(hnswlib_bench
  src/main.cpp
  src/benchmark_index_build.cpp
  src/benchmark_search.cpp
  src/benchmark_concurrent.cpp
  ${COMMON_SOURCES}
)

target_link_libraries(hnswlib_bench
  hnswlib
  arena_hnswlib
  benchmark::benchmark
  pthread
)

target_include_directories(hnswlib_bench PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Unit test executable
set(TEST_SOURCES
  tests/cli_parser_test.cpp
  tests/index_wrapper_test.cpp
  tests/hnswlib_wrapper_test.cpp
  tests/arena_hnswlib_wrapper_test.cpp
  tests/index_manager_test.cpp
)

add_executable(unit_tests
  ${TEST_SOURCES}
  ${COMMON_SOURCES}
)

target_link_libraries(unit_tests
  hnswlib
  arena_hnswlib
  gtest
  gtest_main
  pthread
)

target_include_directories(unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable testing
enable_testing()
include(GoogleTest)
gtest_discover_tests(unit_tests)

# =============================================================================
# Build info
# =============================================================================

message(STATUS "")
message(STATUS "Build configuration:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "")
message(STATUS "Executables:")
message(STATUS "  hnswlib_bench - Unified benchmark with CLI support")
message(STATUS "  unit_tests    - Unit tests")
message(STATUS "")
