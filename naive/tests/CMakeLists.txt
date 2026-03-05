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
project(hnswlib_benchmark_tests VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# Enable AVX for arena-hnswlib SIMD optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -mavx)
  add_compile_options(-Wno-pedantic -Wno-ignored-qualifiers)
  add_compile_options(-fno-tree-vectorize)
endif()

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

# hnswlib (header-only)
add_library(hnswlib INTERFACE)
target_include_directories(hnswlib INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/hnswlib
)

# arena-hnswlib (header-only)
add_library(arena_hnswlib INTERFACE)
target_include_directories(arena_hnswlib INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/arena-hnswlib/include
)

# 源文件
set(TEST_SOURCES
  cli_parser_test.cpp
  index_wrapper_test.cpp
  hnswlib_wrapper_test.cpp
  arena_hnswlib_wrapper_test.cpp
  index_manager_test.cpp
)

set(LIB_SOURCES
  ../src/cli_parser.cpp
  ../src/hnswlib_wrapper.cpp
  ../src/arena_hnswlib_wrapper.cpp
  ../src/index_manager.cpp
  ../src/index_wrapper.cpp
  ../src/dataset_loader.cpp
)

# 创建测试可执行文件
add_executable(unit_tests
  ${TEST_SOURCES}
  ${LIB_SOURCES}
)

target_link_libraries(unit_tests
  hnswlib
  arena_hnswlib
  gtest
  gtest_main
  pthread
)

target_include_directories(unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# 启用测试
enable_testing()
include(GoogleTest)
gtest_discover_tests(unit_tests)
