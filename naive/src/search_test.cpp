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

#include <hnswlib/hnswlib.h>
#include <arena-hnswlib/arena_hnswlib.h>
#include <arena-hnswlib/hnswalg.h>
#include <arena-hnswlib/bruteforce.h>
#include <arena-hnswlib/space_l2.h>
#include <arena-hnswlib/space_ip.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "benchmark_config.h"
#include "dataset_loader.h"

namespace {

// Test mode enumeration
enum class TestMode {
  kBaseline,            // hnswlib
  kDoubleM,             // arena-hnswlib DoubleM mode
  kHeuristicOnly,       // arena-hnswlib HeuristicOnly mode
  kHeuristicPlusClosest // arena-hnswlib HeuristicPlusClosest mode
};

// Convert TestMode to string
std::string TestModeToString(TestMode mode) {
  switch (mode) {
    case TestMode::kBaseline:
      return "hnswlib-baseline";
    case TestMode::kDoubleM:
      return "arena-DoubleM";
    case TestMode::kHeuristicOnly:
      return "arena-HeuristicOnly";
    case TestMode::kHeuristicPlusClosest:
      return "arena-HeuristicPlusClosest";
    default:
      return "Unknown";
  }
}

// Parse TestMode from string
TestMode ParseTestMode(const std::string& str) {
  if (str == "baseline" || str == "hnswlib") {
    return TestMode::kBaseline;
  } else if (str == "DoubleM" || str == "doublem") {
    return TestMode::kDoubleM;
  } else if (str == "HeuristicOnly" || str == "heuristiconly") {
    return TestMode::kHeuristicOnly;
  } else if (str == "HeuristicPlusClosest" || str == "heuristicplusclosest") {
    return TestMode::kHeuristicPlusClosest;
  } else {
    throw std::invalid_argument("Unknown test mode: " + str);
  }
}

// Calculate recall@k
// k must be <= ground_truth size (typically 100 for ann-benchmarks)
double CalculateRecall(
    const std::vector<std::vector<int>>& predicted,
    const std::vector<std::vector<int>>& ground_truth,
    int k) {
  double total_recall = 0.0;
  int num_queries = static_cast<int>(predicted.size());

  for (int i = 0; i < num_queries; ++i) {
    int gt_size = static_cast<int>(ground_truth[i].size());
    int effective_k = std::min(k, gt_size);

    std::set<int> gt_set(ground_truth[i].begin(),
                         ground_truth[i].begin() + effective_k);

    int correct = 0;
    int pred_size = static_cast<int>(predicted[i].size());
    int actual_k = std::min(effective_k, pred_size);
    for (int j = 0; j < actual_k; ++j) {
      if (gt_set.count(predicted[i][j])) {
        ++correct;
      }
    }

    // Use effective_k as denominator to avoid underestimating recall
    total_recall += static_cast<double>(correct) / effective_k;
  }

  return total_recall / num_queries;
}

// Abstract index wrapper for unified interface
class IndexWrapper {
 public:
  virtual ~IndexWrapper() = default;
  virtual void addPoint(const float* data, size_t id) = 0;
  virtual std::vector<int> searchKnn(const float* query, int k, int ef_search) = 0;
  virtual void setEfSearch(int ef_search) = 0;
};

// hnswlib index wrapper
class HnswlibIndexWrapper : public IndexWrapper {
  std::unique_ptr<hnswlib::SpaceInterface<float>> space_;
  std::unique_ptr<hnswlib::HierarchicalNSW<float>> index_;

 public:
  HnswlibIndexWrapper(naive::MetricType metric_type, int dim, 
                      size_t max_elements, int M, int ef_construction)
      : space_(nullptr), index_(nullptr) {
    switch (metric_type) {
      case naive::MetricType::kL2:
        space_ = std::make_unique<hnswlib::L2Space>(dim);
        break;
      case naive::MetricType::kInnerProduct:
        space_ = std::make_unique<hnswlib::InnerProductSpace>(dim);
        break;
    }
    index_ = std::make_unique<hnswlib::HierarchicalNSW<float>>(
        space_.get(), max_elements, M, ef_construction);
  }

  void addPoint(const float* data, size_t id) override {
    index_->addPoint(data, static_cast<hnswlib::labeltype>(id));
  }

  std::vector<int> searchKnn(const float* query, int k, int ef_search) override {
    index_->setEf(ef_search);
    auto result = index_->searchKnn(query, k);
    
    std::vector<int> ids;
    while (!result.empty()) {
      ids.push_back(static_cast<int>(result.top().second));
      result.pop();
    }
    std::reverse(ids.begin(), ids.end());
    return ids;
  }

  void setEfSearch(int ef_search) override {
    index_->setEf(ef_search);
  }
};

// arena-hnswlib index wrapper (template-based)
template<typename SpaceT>
class ArenaHnswlibIndexWrapper : public IndexWrapper {
  SpaceT space_;
  std::unique_ptr<arena_hnswlib::HierarchicalNSW<float, SpaceT>> index_;

 public:
  ArenaHnswlibIndexWrapper(SpaceT space, size_t max_elements, 
                           int M, int ef_construction,
                           arena_hnswlib::Layer0NeighborMode mode)
      : space_(std::move(space)) {
    index_ = std::make_unique<arena_hnswlib::HierarchicalNSW<float, SpaceT>>(
        space_, max_elements, M, ef_construction, 42, mode);
  }

  void addPoint(const float* data, size_t id) override {
    index_->addPoint(data, static_cast<arena_hnswlib::LabelType>(id));
  }

  std::vector<int> searchKnn(const float* query, int k, int ef_search) override {
    index_->setEfSearch(ef_search);
    auto result = index_->searchKnn(query, k);
    
    std::vector<int> ids;
    while (!result.empty()) {
      ids.push_back(static_cast<int>(result.top().second));
      result.pop();
    }
    std::reverse(ids.begin(), ids.end());
    return ids;
  }

  void setEfSearch(int ef_search) override {
    index_->setEfSearch(ef_search);
  }
};

// Create index wrapper based on mode
std::unique_ptr<IndexWrapper> CreateIndex(
    TestMode mode,
    naive::MetricType metric_type,
    int dim,
    size_t max_elements,
    int M,
    int ef_construction) {
  
  switch (mode) {
    case TestMode::kBaseline:
      return std::make_unique<HnswlibIndexWrapper>(
          metric_type, dim, max_elements, M, ef_construction);
    
    case TestMode::kDoubleM:
      if (metric_type == naive::MetricType::kL2) {
        return std::make_unique<ArenaHnswlibIndexWrapper<arena_hnswlib::L2Space<float>>>(
            arena_hnswlib::L2Space<float>(dim), max_elements, M, ef_construction,
            arena_hnswlib::Layer0NeighborMode::kDoubleM);
      } else {
        return std::make_unique<ArenaHnswlibIndexWrapper<arena_hnswlib::InnerProductSpace<float>>>(
            arena_hnswlib::InnerProductSpace<float>(dim), max_elements, M, ef_construction,
            arena_hnswlib::Layer0NeighborMode::kDoubleM);
      }
    
    case TestMode::kHeuristicOnly:
      if (metric_type == naive::MetricType::kL2) {
        return std::make_unique<ArenaHnswlibIndexWrapper<arena_hnswlib::L2Space<float>>>(
            arena_hnswlib::L2Space<float>(dim), max_elements, M, ef_construction,
            arena_hnswlib::Layer0NeighborMode::kHeuristicOnly);
      } else {
        return std::make_unique<ArenaHnswlibIndexWrapper<arena_hnswlib::InnerProductSpace<float>>>(
            arena_hnswlib::InnerProductSpace<float>(dim), max_elements, M, ef_construction,
            arena_hnswlib::Layer0NeighborMode::kHeuristicOnly);
      }
    
    case TestMode::kHeuristicPlusClosest:
      if (metric_type == naive::MetricType::kL2) {
        return std::make_unique<ArenaHnswlibIndexWrapper<arena_hnswlib::L2Space<float>>>(
            arena_hnswlib::L2Space<float>(dim), max_elements, M, ef_construction,
            arena_hnswlib::Layer0NeighborMode::kHeuristicPlusClosest);
      } else {
        return std::make_unique<ArenaHnswlibIndexWrapper<arena_hnswlib::InnerProductSpace<float>>>(
            arena_hnswlib::InnerProductSpace<float>(dim), max_elements, M, ef_construction,
            arena_hnswlib::Layer0NeighborMode::kHeuristicPlusClosest);
      }
    
    default:
      throw std::invalid_argument("Unknown test mode");
  }
}

// Build index in a thread
void BuildIndexThread(
    IndexWrapper* index,
    const naive::Dataset& dataset,
    std::atomic<int>& progress_counter,
    std::mutex& progress_mutex,
    std::condition_variable& progress_cv) {
  
  for (int i = 0; i < dataset.train_size; ++i) {
    index->addPoint(dataset.train_vectors[i].data(), i);
    
    // Update progress every 10000 points
    if (i % 10000 == 0) {
      std::lock_guard<std::mutex> lock(progress_mutex);
      progress_counter = i;
      progress_cv.notify_one();
    }
  }
  
  // Final update
  {
    std::lock_guard<std::mutex> lock(progress_mutex);
    progress_counter = dataset.train_size;
    progress_cv.notify_one();
  }
}

// Search performance test result
struct SearchResult {
  double recall;
  double latency_us;
  double qps;
};

// Single-threaded search test
SearchResult TestSingleThreadedSearch(
    IndexWrapper* index,
    const naive::Dataset& dataset,
    int k,
    int ef_search,
    std::vector<std::vector<int>>& results) {
  
  results.resize(dataset.test_size);
  
  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < dataset.test_size; ++i) {
    results[i] = index->searchKnn(dataset.test_vectors[i].data(), k, ef_search);
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  
  SearchResult result;
  result.recall = CalculateRecall(results, dataset.ground_truth, k);
  result.latency_us = static_cast<double>(duration.count()) / dataset.test_size;
  result.qps = dataset.test_size * 1000000.0 / duration.count();
  
  return result;
}

// Multi-threaded search test
SearchResult TestMultiThreadedSearch(
    IndexWrapper* index,
    const naive::Dataset& dataset,
    int k,
    int ef_search,
    int num_threads,
    std::vector<std::vector<int>>& results) {
  
  results.resize(dataset.test_size);
  
  auto start = std::chrono::high_resolution_clock::now();
  
  std::vector<std::thread> threads;
  std::atomic<int> query_counter{0};
  
  auto worker = [&]() {
    while (true) {
      int query_idx = query_counter.fetch_add(1);
      if (query_idx >= dataset.test_size) {
        break;
      }
      
      results[query_idx] = index->searchKnn(
          dataset.test_vectors[query_idx].data(), k, ef_search);
    }
  };
  
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(worker);
  }
  
  for (auto& thread : threads) {
    thread.join();
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  
  SearchResult result;
  result.recall = CalculateRecall(results, dataset.ground_truth, k);
  result.latency_us = static_cast<double>(duration.count()) / dataset.test_size;
  result.qps = dataset.test_size * 1000000.0 / duration.count();
  
  return result;
}

}  // namespace

int main(int argc, char** argv) {
  // Parse arguments
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <data_dir> <modes> [output_file] [k]\n"
              << "\n"
              << "Arguments:\n"
              << "  data_dir     Path to dataset directory\n"
              << "               - data/sift-128-euclidean\n"
              << "               - data/glove\n"
              << "               - data/gist\n"
              << "\n"
              << "  modes        Comma-separated list of test modes:\n"
              << "               - baseline (hnswlib)\n"
              << "               - DoubleM\n"
              << "               - HeuristicOnly\n"
              << "               - HeuristicPlusClosest\n"
              << "               Example: baseline,DoubleM,HeuristicOnly\n"
              << "\n"
              << "  output_file  Path to output file (default: /tmp/benchmark_results.txt)\n"
              << "\n"
              << "  k            Number of nearest neighbors to search (default: 10)\n"
              << "               Must be <= groundtruth size (typically 100)\n"
              << "\n"
              << "Example:\n"
              << "  " << argv[0] << " data/sift-128-euclidean baseline,DoubleM,HeuristicOnly\n"
              << "  " << argv[0] << " data/sift-128-euclidean baseline,HeuristicOnly /tmp/results.txt 100\n";
    return 1;
  }

  std::string data_dir = argv[1];
  std::string modes_str = argv[2];
  std::string output_file = (argc > 3) ? argv[3] : "/tmp/benchmark_results.txt";
  int k = (argc > 4) ? std::stoi(argv[4]) : 10;

  // Parse modes
  std::vector<TestMode> test_modes;
  std::stringstream ss(modes_str);
  std::string mode_str;
  while (std::getline(ss, mode_str, ',')) {
    // Trim whitespace
    mode_str.erase(0, mode_str.find_first_not_of(" \t\n\r\f\v"));
    mode_str.erase(mode_str.find_last_not_of(" \t\n\r\f\v") + 1);
    
    test_modes.push_back(ParseTestMode(mode_str));
  }

  // Load dataset
  std::cout << "Loading dataset from " << data_dir << "..." << std::endl;
  naive::Dataset dataset = naive::DatasetLoader::Load(data_dir);

  // Validate k parameter
  if (k <= 0 || k > dataset.k) {
    std::cerr << "Error: k must be in range [1, " << dataset.k << "]"
              << ", got " << k << std::endl;
    return 1;
  }

  std::cout << "\nDataset: " << dataset.name << std::endl;
  std::cout << "  Train vectors: " << dataset.train_size << std::endl;
  std::cout << "  Test vectors: " << dataset.test_size << std::endl;
  std::cout << "  Dimension: " << dataset.dimension << std::endl;
  std::cout << "  Ground truth k: " << dataset.k << std::endl;
  std::cout << "  Metric: "
            << (dataset.metric_type == naive::MetricType::kL2
                   ? "L2"
                   : "InnerProduct")
            << std::endl;

  std::cout << "\nTest parameters:" << std::endl;
  std::cout << "  k (top-k neighbors): " << k << std::endl;

  std::cout << "\nTest modes:" << std::endl;
  for (const auto& mode : test_modes) {
    std::cout << "  - " << TestModeToString(mode) << std::endl;
  }

  // Configuration
  naive::BenchmarkConfig config;
  config.k = k;  // Use user-specified k
  std::vector<int> ef_search_values = {100, 200, 500, 1000};
  std::vector<int> num_threads_values = {1, 2, 4, 8};

  // Build all indexes in parallel
  std::cout << "\n========================================" << std::endl;
  std::cout << "[Phase 1] Building indexes in parallel..." << std::endl;
  std::cout << "========================================" << std::endl;

  std::vector<std::unique_ptr<IndexWrapper>> indexes(test_modes.size());
  std::vector<std::thread> build_threads(test_modes.size());
  std::vector<std::atomic<int>> progress_counters(test_modes.size());
  std::vector<std::mutex> progress_mutexes(test_modes.size());
  std::vector<std::condition_variable> progress_cvs(test_modes.size());

  auto build_start = std::chrono::high_resolution_clock::now();

  // Start building each index in a separate thread
  for (size_t i = 0; i < test_modes.size(); ++i) {
    std::cout << "Creating index: " << TestModeToString(test_modes[i]) << std::endl;
    
    indexes[i] = CreateIndex(
        test_modes[i],
        dataset.metric_type,
        dataset.dimension,
        dataset.train_size,
        config.M,
        config.ef_construction);
    
    progress_counters[i] = 0;
    
    build_threads[i] = std::thread(
        BuildIndexThread,
        indexes[i].get(),
        std::cref(dataset),
        std::ref(progress_counters[i]),
        std::ref(progress_mutexes[i]),
        std::ref(progress_cvs[i]));
  }

  // Monitor progress
  bool all_done = false;
  while (!all_done) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    all_done = true;
    std::cout << "\rProgress: ";
    for (size_t i = 0; i < test_modes.size(); ++i) {
      int progress = progress_counters[i].load();
      double percent = 100.0 * progress / dataset.train_size;
      
      std::cout << TestModeToString(test_modes[i]).substr(0, 10) << " "
                << std::fixed << std::setprecision(1) << percent << "% ";
      
      if (progress < dataset.train_size) {
        all_done = false;
      }
    }
    std::cout << std::flush;
  }
  std::cout << std::endl;

  // Wait for all threads to complete
  for (auto& thread : build_threads) {
    thread.join();
  }

  auto build_end = std::chrono::high_resolution_clock::now();
  auto build_duration = std::chrono::duration_cast<std::chrono::seconds>(
      build_end - build_start);

  std::cout << "\nAll indexes built in " << build_duration.count() << " seconds" << std::endl;

  // Test each index
  std::cout << "\n========================================" << std::endl;
  std::cout << "[Phase 2] Testing search performance..." << std::endl;
  std::cout << "========================================" << std::endl;

  // Open output file
  std::ofstream out(output_file);
  if (!out.is_open()) {
    std::cerr << "Error: Cannot open output file: " << output_file << std::endl;
    return 1;
  }

  out << "Test Started: " << __DATE__ << " " << __TIME__ << "\n";
  out << "Dataset: " << data_dir << "\n";
  out << "Modes: " << modes_str << "\n";
  out << "k (top-k neighbors): " << k << "\n";
  out << "========================================\n\n";

  // Test each mode
  for (size_t i = 0; i < test_modes.size(); ++i) {
    std::cout << "\n[" << (i+1) << "/" << test_modes.size() << "] "
              << "Testing: " << TestModeToString(test_modes[i]) << std::endl;
    
    out << "========== " << TestModeToString(test_modes[i]) << " ==========\n";
    
    // Single-threaded search tests
    std::cout << "  Single-threaded search:" << std::endl;
    out << "Single-threaded:\n";
    
    for (int ef_search : ef_search_values) {
      std::vector<std::vector<int>> results;
      SearchResult result = TestSingleThreadedSearch(
          indexes[i].get(), dataset, config.k, ef_search, results);
      
      std::cout << "    ef_search=" << ef_search
                << " | recall=" << std::fixed << std::setprecision(2) 
                << (result.recall * 100) << "%"
                << " | latency=" << std::setprecision(0) << result.latency_us 
                << " us/query"
                << " | qps=" << std::setprecision(1) << result.qps << std::endl;
      
      out << "  ef_search=" << ef_search
          << ", recall=" << std::fixed << std::setprecision(4) 
          << (result.recall * 100) << "%"
          << ", latency=" << std::setprecision(2) << result.latency_us 
          << " us/query"
          << ", qps=" << std::setprecision(2) << result.qps << "\n";
    }
    
    // Multi-threaded search tests
    std::cout << "\n  Multi-threaded search (ef_search=1000):" << std::endl;
    out << "\nMulti-threaded (ef_search=1000):\n";
    
    for (int num_threads : num_threads_values) {
      std::vector<std::vector<int>> results;
      SearchResult result = TestMultiThreadedSearch(
          indexes[i].get(), dataset, config.k, 1000, num_threads, results);
      
      std::cout << "    threads=" << num_threads
                << " | recall=" << std::fixed << std::setprecision(2) 
                << (result.recall * 100) << "%"
                << " | latency=" << std::setprecision(0) << result.latency_us 
                << " us/query"
                << " | qps=" << std::setprecision(1) << result.qps << std::endl;
      
      out << "  threads=" << num_threads
          << ", recall=" << std::fixed << std::setprecision(4) 
          << (result.recall * 100) << "%"
          << ", latency=" << std::setprecision(2) << result.latency_us 
          << " us/query"
          << ", qps=" << std::setprecision(2) << result.qps << "\n";
    }
    
    out << "\n";
    std::cout << std::endl;
  }

  out << "Test Completed: " << __DATE__ << " " << __TIME__ << "\n";
  out.close();

  std::cout << "========================================" << std::endl;
  std::cout << "All tests completed!" << std::endl;
  std::cout << "Results saved to: " << output_file << std::endl;
  std::cout << "========================================" << std::endl;

  return 0;
}
