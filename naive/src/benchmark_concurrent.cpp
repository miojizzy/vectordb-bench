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
#include <atomic>
#include <thread>
#include <vector>

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

// Generic concurrent search benchmark template
template<naive::IndexType Type>
static void BM_Search_Concurrent(benchmark::State& state) {
  int M = state.range(0);
  int ef_construction = state.range(1);
  int ef_search = state.range(2);
  int num_threads = state.range(3);
  int k = g_config.k;
  
  // Get or create index
  auto* index = naive::internal::IndexManager::GetOrCreateIndex(
      Type, g_dataset, M, ef_construction);
  
  index->SetEfSearch(ef_search);
  
  std::atomic<int> query_counter{0};
  
  // Thread worker function
  auto worker = [&](int thread_id) {
    int queries_per_thread = g_dataset.test_size / num_threads;
    int start_idx = thread_id * queries_per_thread;
    int end_idx = (thread_id == num_threads - 1) 
                  ? g_dataset.test_size 
                  : start_idx + queries_per_thread;
    
    std::vector<std::vector<int>> local_results(end_idx - start_idx);
    
    for (int i = start_idx; i < end_idx; ++i) {
      local_results[i - start_idx] = index->SearchKnn(
          g_dataset.test_vectors[i].data(), k, ef_search);
      query_counter++;
    }
  };
  
  for (auto _ : state) {
    query_counter = 0;
    
    // Launch threads
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
      threads.emplace_back(worker, t);
    }
    
    // Wait for completion
    for (auto& thread : threads) {
      thread.join();
    }
    
    state.SetItemsProcessed(query_counter.load());
  }
  
  // Calculate recall (approximate, from first thread's results)
  std::vector<std::vector<int>> results(g_dataset.test_size);
  for (int i = 0; i < g_dataset.test_size; ++i) {
    results[i] = index->SearchKnn(
        g_dataset.test_vectors[i].data(), k, ef_search);
  }
  double recall = naive::CalculateRecall(results, g_dataset.ground_truth, k);
  
  state.counters["recall"] = recall * 100;
  state.counters["threads"] = num_threads;
  state.counters["ef_search"] = ef_search;
  state.counters["M"] = M;
  state.counters["ef_construction"] = ef_construction;
}

// Register benchmarks for all index types
BENCHMARK_TEMPLATE(BM_Search_Concurrent, naive::IndexType::kHnswlib)
    ->Name("BM_Search_Concurrent/HNSW")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {100},                       // ef_search (high for better recall)
        {1, 2, 4, 8}                 // num_threads
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Search_Concurrent, naive::IndexType::kArenaDoubleM)
    ->Name("BM_Search_Concurrent/DoubleM")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {100},                       // ef_search
        {1, 2, 4, 8}                 // num_threads
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Search_Concurrent, naive::IndexType::kArenaHeuristicOnly)
    ->Name("BM_Search_Concurrent/HeurOnly")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {100},                       // ef_search
        {1, 2, 4, 8}                 // num_threads
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Search_Concurrent, naive::IndexType::kArenaHeuristicPlusClosest)
    ->Name("BM_Search_Concurrent/HeurClosest")
    ->ArgsProduct({
        {16, 32},                    // M
        {200},                        // ef_construction
        {100},                       // ef_search
        {1, 2, 4, 8}                 // num_threads
    })
    ->Unit(benchmark::kMicrosecond);

}  // namespace
