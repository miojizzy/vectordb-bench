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

// Generic search benchmark template
template<naive::IndexType Type>
static void BM_Search(benchmark::State& state) {
  int M = state.range(0);
  int ef_construction = state.range(1);
  int ef_search = state.range(2);
  int k = g_config.k;
  
  // Get or create index
  auto* index = naive::internal::IndexManager::GetOrCreateIndex(
      Type, g_dataset, M, ef_construction);
  
  index->SetEfSearch(ef_search);
  
  std::vector<std::vector<int>> results(g_dataset.test_size);
  
  for (auto _ : state) {
    for (int i = 0; i < g_dataset.test_size; ++i) {
      results[i] = index->SearchKnn(
          g_dataset.test_vectors[i].data(), k, ef_search);
    }
  }
  
  double recall = naive::CalculateRecall(results, g_dataset.ground_truth, k);
  
  state.counters["recall"] = recall * 100;
  state.counters["ef_search"] = ef_search;
  state.counters["M"] = M;
  state.counters["ef_construction"] = ef_construction;
  state.SetItemsProcessed(state.iterations() * g_dataset.test_size);
}

// Register benchmarks for all index types
// Using ArgsProduct for all combinations
BENCHMARK_TEMPLATE(BM_Search, naive::IndexType::kHnswlib)
    ->Name("BM_Search/HNSW")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {10, 20, 50, 100}        // ef_search
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Search, naive::IndexType::kArenaDoubleM)
    ->Name("BM_Search/DoubleM")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {10, 20, 50, 100}        // ef_search
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Search, naive::IndexType::kArenaHeuristicOnly)
    ->Name("BM_Search/HeurOnly")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {10, 20, 50, 100}        // ef_search
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Search, naive::IndexType::kArenaHeuristicPlusClosest)
    ->Name("BM_Search/HeurClosest")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {10, 20, 50, 100}        // ef_search
    })
    ->Unit(benchmark::kMicrosecond);

}  // namespace
