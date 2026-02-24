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
project(hnswlib_benchmark VERSION 0.1.0 LANGUAGES CXX)

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

# Benchmark executable
add_executable(hnswlib_bench
  src/main.cpp
  src/dataset_loader.cpp
)

target_link_libraries(hnswlib_bench
  hnswlib
  arena_hnswlib
  benchmark::benchmark
)

target_include_directories(hnswlib_bench PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
