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
#include <iostream>

#include "cli_parser.h"
#include "dataset_loader.h"
#include "index_manager.h"

namespace naive {

// Global dataset and config (shared with benchmark files)
Dataset g_dataset;
CliConfig g_config;

}  // namespace naive

namespace {

// Import for convenience
using naive::g_dataset;
using naive::g_config;

// Check if benchmark filter contains index build tests
bool HasIndexBuildTest(const std::string& filter) {
  return filter.empty() || 
         filter.find("IndexBuild") != std::string::npos ||
         filter.find("*") != std::string::npos;
}

// Prebuild indexes if needed
void PrebuildIndexesIfNeeded() {
  bool has_build_test = HasIndexBuildTest(g_config.benchmark_filter);
  
  if (!has_build_test) {
    // No build test, need to prebuild indexes
    std::cout << "\nNo index build test detected, prebuilding indexes..." << std::endl;
    naive::internal::IndexManager::BuildAllIndexes(
        g_dataset, g_config, g_config.build_index_parallel);
    std::cout << "Indexes built successfully!" << std::endl;
  } else {
    std::cout << "\nIndex build test detected, will build indexes during benchmark." << std::endl;
  }
}

}  // namespace

int main(int argc, char** argv) {
  // Parse command line arguments
  try {
    g_config = naive::CliParser::Parse(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
    naive::CliParser::PrintHelp(argv[0]);
    return 1;
  }
  
  // Load dataset
  std::cout << "Loading dataset from " << g_config.data_dir << "..." << std::endl;
  try {
    g_dataset = naive::DatasetLoader::Load(g_config.data_dir);
  } catch (const std::exception& e) {
    std::cerr << "Error loading dataset: " << e.what() << std::endl;
    return 1;
  }
  
  // Print dataset info
  std::cout << "\nDataset: " << g_dataset.name << std::endl;
  std::cout << "  Train vectors: " << g_dataset.train_size << std::endl;
  std::cout << "  Test vectors: " << g_dataset.test_size << std::endl;
  std::cout << "  Dimension: " << g_dataset.dimension << std::endl;
  std::cout << "  Metric: " 
            << (g_dataset.metric_type == naive::MetricType::kL2 ? "L2" : "IP")
            << std::endl;
  
  // Print test configuration
  std::cout << "\nTest Configuration:" << std::endl;
  std::cout << "  Index types: ";
  for (auto type : g_config.index_types) {
    std::cout << naive::CliParser::IndexTypeToString(type) << " ";
  }
  std::cout << std::endl;
  
  std::cout << "  M values: ";
  for (int M : g_config.M_values) {
    std::cout << M << " ";
  }
  std::cout << std::endl;
  
  std::cout << "  ef_construction values: ";
  for (int ef : g_config.ef_construction_values) {
    std::cout << ef << " ";
  }
  std::cout << std::endl;
  
  std::cout << "  ef_search values: ";
  for (int ef : g_config.ef_search_values) {
    std::cout << ef << " ";
  }
  std::cout << std::endl;
  
  std::cout << "  num_threads values: ";
  for (int t : g_config.num_threads_values) {
    std::cout << t << " ";
  }
  std::cout << std::endl;
  
  std::cout << "  k (top-k): " << g_config.k << std::endl;
  std::cout << "  Build parallel: " << (g_config.build_index_parallel ? "yes" : "no") << std::endl;
  
  // Prebuild indexes if needed
  PrebuildIndexesIfNeeded();
  
  // Prepare Google Benchmark arguments
  // Keep strings alive in a vector
  std::vector<std::string> bench_args_storage;
  std::vector<char*> bench_argv;
  bench_argv.push_back(argv[0]);
  
  // Add benchmark filter
  if (!g_config.benchmark_filter.empty()) {
    bench_args_storage.push_back("--benchmark_filter=" + g_config.benchmark_filter);
    bench_argv.push_back(const_cast<char*>(bench_args_storage.back().c_str()));
  }
  
  // Add benchmark output file
  if (!g_config.benchmark_out.empty()) {
    bench_args_storage.push_back("--benchmark_out=" + g_config.benchmark_out);
    bench_argv.push_back(const_cast<char*>(bench_args_storage.back().c_str()));
  }
  
  // Add benchmark format
  if (!g_config.benchmark_format.empty()) {
    bench_args_storage.push_back("--benchmark_format=" + g_config.benchmark_format);
    bench_argv.push_back(const_cast<char*>(bench_args_storage.back().c_str()));
  }
  
  // Add output file if specified
  if (!g_config.output_file.empty()) {
    bench_args_storage.push_back("--benchmark_out=" + g_config.output_file);
    bench_argv.push_back(const_cast<char*>(bench_args_storage.back().c_str()));
  }
  
  // Run benchmarks
  std::cout << "\n========================================" << std::endl;
  std::cout << "Running benchmarks..." << std::endl;
  std::cout << "========================================\n" << std::endl;
  
  int bench_argc = static_cast<int>(bench_argv.size());
  benchmark::Initialize(&bench_argc, bench_argv.data());
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  
  // Cleanup
  naive::internal::IndexManager::Clear();
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "All benchmarks completed!" << std::endl;
  std::cout << "========================================" << std::endl;
  
  return 0;
}
