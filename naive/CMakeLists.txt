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

# Google Benchmark
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

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

# Common source files (shared by all executables)
set(COMMON_SOURCES
  src/dataset_loader.cpp
  src/cli_parser.cpp
  src/index_wrapper.cpp
  src/hnswlib_wrapper.cpp
  src/arena_hnswlib_wrapper.cpp
  src/index_manager.cpp
)

# Unified benchmark executable
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

# Print build info
message(STATUS "")
message(STATUS "Build configuration:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "")
message(STATUS "Executables:")
message(STATUS "  hnswlib_bench - Unified benchmark with CLI support")
message(STATUS "")
