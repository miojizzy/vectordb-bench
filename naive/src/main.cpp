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
#include <hnswlib/hnswlib.h>
#include <arena-hnswlib/arena_hnswlib.h>
#include <arena-hnswlib/hnswalg.h>
#include <arena-hnswlib/space_l2.h>
#include <arena-hnswlib/space_ip.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>

#include "benchmark_config.h"
#include "dataset_loader.h"

namespace {

// Global dataset (loaded once)
naive::Dataset g_dataset;

// Benchmark configuration
naive::BenchmarkConfig g_config;

// Calculate recall@k
double CalculateRecall(
    const std::vector<std::vector<int>>& predicted,
    const std::vector<std::vector<int>>& ground_truth,
    int k) {
  double total_recall = 0.0;
  int num_queries = static_cast<int>(predicted.size());

  for (int i = 0; i < num_queries; ++i) {
    std::set<int> gt_set(ground_truth[i].begin(),
                         ground_truth[i].begin() + std::min(k,
                             static_cast<int>(ground_truth[i].size())));

    int correct = 0;
    int actual_k = std::min(k, static_cast<int>(predicted[i].size()));
    for (int j = 0; j < actual_k; ++j) {
      if (gt_set.count(predicted[i][j])) {
        ++correct;
      }
    }

    total_recall += static_cast<double>(correct) / k;
  }

  return total_recall / num_queries;
}

// ============================================================================
// hnswlib benchmarks
// ============================================================================

std::unique_ptr<hnswlib::SpaceInterface<float>> CreateHnswlibSpace(
    naive::MetricType metric_type, int dim) {
  switch (metric_type) {
    case naive::MetricType::kL2:
      return std::make_unique<hnswlib::L2Space>(dim);
    case naive::MetricType::kInnerProduct:
      return std::make_unique<hnswlib::InnerProductSpace>(dim);
    default:
      throw std::runtime_error("Unknown metric type");
  }
}

// Benchmark: hnswlib Index Construction
static void BM_Hnswlib_IndexConstruction(benchmark::State& state) {
  int M = state.range(0);
  int ef_construction = state.range(1);

  for (auto _ : state) {
    auto space = CreateHnswlibSpace(g_dataset.metric_type, g_dataset.dimension);
    hnswlib::HierarchicalNSW<float> index(
        space.get(), g_dataset.train_size, M, ef_construction);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < g_dataset.train_size; ++i) {
      index.addPoint(g_dataset.train_vectors[i].data(),
                     static_cast<hnswlib::labeltype>(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);

    // Use escape to prevent optimization without DoNotOptimize issues
    auto* ptr = &index;
    benchmark::DoNotOptimize(ptr);

    state.SetItemsProcessed(g_dataset.train_size);
    state.counters["duration_ms"] = duration.count();
  }
}
BENCHMARK(BM_Hnswlib_IndexConstruction)
    ->Args({16, 200})
    ->Args({32, 200})
    ->Unit(benchmark::kMillisecond);

// Benchmark: hnswlib Search
static void BM_Hnswlib_Search(benchmark::State& state) {
  int ef_search = state.range(0);
  int k = g_config.k;

  // Build index
  auto space = CreateHnswlibSpace(g_dataset.metric_type, g_dataset.dimension);
  hnswlib::HierarchicalNSW<float> index(
      space.get(), g_dataset.train_size, g_config.M, g_config.ef_construction);

  for (int i = 0; i < g_dataset.train_size; ++i) {
    index.addPoint(g_dataset.train_vectors[i].data(),
                   static_cast<hnswlib::labeltype>(i));
  }

  index.setEf(ef_search);

  std::vector<std::vector<int>> all_results(g_dataset.test_size);

  for (auto _ : state) {
    for (int i = 0; i < g_dataset.test_size; ++i) {
      auto result = index.searchKnn(g_dataset.test_vectors[i].data(), k);

      all_results[i].clear();
      while (!result.empty()) {
        all_results[i].push_back(static_cast<int>(result.top().second));
        result.pop();
      }
      std::reverse(all_results[i].begin(), all_results[i].end());
    }
  }

  double recall = CalculateRecall(all_results, g_dataset.ground_truth, k);

  state.SetItemsProcessed(state.iterations() * g_dataset.test_size);
  state.counters["recall"] = recall * 100;
  state.counters["ef_search"] = ef_search;
}
BENCHMARK(BM_Hnswlib_Search)
    ->Args({50})
    ->Args({100})
    ->Args({200})
    ->Args({500})
    ->Unit(benchmark::kMicrosecond);

// ============================================================================
// arena-hnswlib benchmarks
// ============================================================================

arena_hnswlib::SpacePtr<float> CreateArenaHnswlibSpace(
    naive::MetricType metric_type, int dim) {
  switch (metric_type) {
    case naive::MetricType::kL2:
      return std::make_unique<arena_hnswlib::L2Space<float>>(dim);
    case naive::MetricType::kInnerProduct:
      return std::make_unique<arena_hnswlib::InnerProductSpace<float>>(dim);
    default:
      throw std::runtime_error("Unknown metric type");
  }
}

// Benchmark: arena-hnswlib Index Construction
static void BM_ArenaHnswlib_IndexConstruction(benchmark::State& state) {
  int M = state.range(0);
  int ef_construction = state.range(1);

  for (auto _ : state) {
    auto space = CreateArenaHnswlibSpace(g_dataset.metric_type,
                                          g_dataset.dimension);
    arena_hnswlib::HierarchicalNSW<float> index(
        std::move(space), g_dataset.train_size, M, ef_construction);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < g_dataset.train_size; ++i) {
      index.addPoint(g_dataset.train_vectors[i].data(),
                     static_cast<arena_hnswlib::LabelType>(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);

    // Use escape to prevent optimization without DoNotOptimize issues
    auto* ptr = &index;
    benchmark::DoNotOptimize(ptr);

    state.SetItemsProcessed(g_dataset.train_size);
    state.counters["duration_ms"] = duration.count();
  }
}
BENCHMARK(BM_ArenaHnswlib_IndexConstruction)
    ->Args({16, 200})
    ->Args({32, 200})
    ->Unit(benchmark::kMillisecond);

// Benchmark: arena-hnswlib Search
static void BM_ArenaHnswlib_Search(benchmark::State& state) {
  int ef_search = state.range(0);
  int k = g_config.k;

  // Build index
  auto space = CreateArenaHnswlibSpace(g_dataset.metric_type,
                                        g_dataset.dimension);
  arena_hnswlib::HierarchicalNSW<float> index(
      std::move(space), g_dataset.train_size, g_config.M, g_config.ef_construction);

  for (int i = 0; i < g_dataset.train_size; ++i) {
    index.addPoint(g_dataset.train_vectors[i].data(),
                   static_cast<arena_hnswlib::LabelType>(i));
  }

  index.setEfSearch(ef_search);

  std::vector<std::vector<int>> all_results(g_dataset.test_size);

  for (auto _ : state) {
    for (int i = 0; i < g_dataset.test_size; ++i) {
      auto result = index.searchKnn(g_dataset.test_vectors[i].data(), k);

      all_results[i].clear();
      while (!result.empty()) {
        all_results[i].push_back(static_cast<int>(result.top().second));
        result.pop();
      }
      std::reverse(all_results[i].begin(), all_results[i].end());
    }
  }

  double recall = CalculateRecall(all_results, g_dataset.ground_truth, k);

  state.SetItemsProcessed(state.iterations() * g_dataset.test_size);
  state.counters["recall"] = recall * 100;
  state.counters["ef_search"] = ef_search;
}
BENCHMARK(BM_ArenaHnswlib_Search)
    ->Args({50})
    ->Args({100})
    ->Args({200})
    ->Args({500})
    ->Unit(benchmark::kMicrosecond);

// ============================================================================
// Single Query Latency benchmarks
// ============================================================================

static void BM_Hnswlib_SingleQuery(benchmark::State& state) {
  int ef_search = state.range(0);
  int k = g_config.k;

  auto space = CreateHnswlibSpace(g_dataset.metric_type, g_dataset.dimension);
  hnswlib::HierarchicalNSW<float> index(
      space.get(), g_dataset.train_size, g_config.M, g_config.ef_construction);

  for (int i = 0; i < g_dataset.train_size; ++i) {
    index.addPoint(g_dataset.train_vectors[i].data(),
                   static_cast<hnswlib::labeltype>(i));
  }

  index.setEf(ef_search);

  int query_idx = 0;

  for (auto _ : state) {
    auto result = index.searchKnn(g_dataset.test_vectors[query_idx].data(), k);
    benchmark::DoNotOptimize(result);

    query_idx = (query_idx + 1) % g_dataset.test_size;
  }
}
BENCHMARK(BM_Hnswlib_SingleQuery)
    ->Args({100})
    ->Args({200})
    ->Unit(benchmark::kNanosecond);

static void BM_ArenaHnswlib_SingleQuery(benchmark::State& state) {
  int ef_search = state.range(0);
  int k = g_config.k;

  auto space = CreateArenaHnswlibSpace(g_dataset.metric_type,
                                        g_dataset.dimension);
  arena_hnswlib::HierarchicalNSW<float> index(
      std::move(space), g_dataset.train_size, g_config.M, g_config.ef_construction);

  for (int i = 0; i < g_dataset.train_size; ++i) {
    index.addPoint(g_dataset.train_vectors[i].data(),
                   static_cast<arena_hnswlib::LabelType>(i));
  }

  index.setEfSearch(ef_search);

  int query_idx = 0;

  for (auto _ : state) {
    auto result = index.searchKnn(g_dataset.test_vectors[query_idx].data(), k);
    benchmark::DoNotOptimize(result);

    query_idx = (query_idx + 1) % g_dataset.test_size;
  }
}
BENCHMARK(BM_ArenaHnswlib_SingleQuery)
    ->Args({100})
    ->Args({200})
    ->Unit(benchmark::kNanosecond);

}  // namespace

// Main
int main(int argc, char** argv) {
  // Parse dataset argument
  std::string data_dir = "data/sift";

  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [data_dir]\n"
                << "\n"
                << "Arguments:\n"
                << "  data_dir    Path to dataset directory\n"
                << "              - data/sift    (SIFT, L2 distance)\n"
                << "              - data/glove   (GloVe, InnerProduct)\n"
                << "\n"
                << "Default: data/sift\n"
                << "\n"
                << "Compares hnswlib vs arena-hnswlib performance.\n";
      return 0;
    }
    data_dir = arg;
  }

  // Load dataset
  std::cout << "Loading dataset from " << data_dir << "..." << std::endl;
  g_dataset = naive::DatasetLoader::Load(data_dir);

  std::cout << "\nDataset: " << g_dataset.name << std::endl;
  std::cout << "  Train vectors: " << g_dataset.train_size << std::endl;
  std::cout << "  Test vectors: " << g_dataset.test_size << std::endl;
  std::cout << "  Dimension: " << g_dataset.dimension << std::endl;
  std::cout << "  Ground truth k: " << g_dataset.k << std::endl;
  std::cout << "  Metric: "
            << (g_dataset.metric_type == naive::MetricType::kL2
                   ? "L2"
                   : "InnerProduct")
            << std::endl;

  std::cout << "\nComparing: hnswlib vs arena-hnswlib\n" << std::endl;

  // Run benchmarks
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
}
