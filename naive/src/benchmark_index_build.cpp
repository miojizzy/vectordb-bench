// Copyright 2026 VectorDBBench Authors
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <benchmark/benchmark.h>
#include <chrono>

#include "cli_parser.h"
#include "dataset_loader.h"
#include "index_manager.h"
#include "index_wrapper.h"

namespace naive {

// Global variables shared with main.cpp
extern Dataset g_dataset;
extern CliConfig g_config;

}  // namespace naive

namespace {

// Import for convenience
using naive::g_dataset;
using naive::g_config;

// Generic index build benchmark template
template<naive::IndexType Type>
static void BM_IndexBuild_SingleThread(benchmark::State& state) {
  int M = state.range(0);
  int ef_construction = state.range(1);
  
  for (auto _ : state) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Build and cache index
    auto* index = naive::internal::IndexManager::BuildAndCacheIndex(
        Type, g_dataset, M, ef_construction);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    benchmark::DoNotOptimize(index);
    
    state.counters["duration_ms"] = duration.count();
    state.counters["index_size_mb"] = index->GetIndexSize() / (1024.0 * 1024.0);
    state.SetItemsProcessed(g_dataset.train_size);
  }
}

// Register benchmarks for all index types
BENCHMARK_TEMPLATE(BM_IndexBuild_SingleThread, naive::IndexType::kHnswlib)
    ->Name("BM_IndexBuild/HNSW")
    ->Args({16, 200})
    ->Args({32, 200})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_IndexBuild_SingleThread, naive::IndexType::kArenaDoubleM)
    ->Name("BM_IndexBuild/DoubleM")
    ->Args({16, 200})
    ->Args({32, 200})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_IndexBuild_SingleThread, naive::IndexType::kArenaHeuristicOnly)
    ->Name("BM_IndexBuild/HeurOnly")
    ->Args({16, 200})
    ->Args({32, 200})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_IndexBuild_SingleThread, naive::IndexType::kArenaHeuristicPlusClosest)
    ->Name("BM_IndexBuild/HeurClosest")
    ->Args({16, 200})
    ->Args({32, 200})
    ->Unit(benchmark::kMillisecond);

}  // namespace
