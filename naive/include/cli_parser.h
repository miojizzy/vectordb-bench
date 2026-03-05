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

#pragma once

#include <string>
#include <vector>

namespace naive {

// Index type enumeration
enum class IndexType {
  kHnswlib,
  kArenaDoubleM,
  kArenaHeuristicOnly,
  kArenaHeuristicPlusClosest
};

// Command line configuration
struct CliConfig {
  // Dataset related
  std::string data_dir;              // Dataset directory
  std::string output_file;           // Output file
  
  // Index related
  std::vector<IndexType> index_types; // Index types to test
  std::vector<int> M_values;          // HNSW M parameter values
  std::vector<int> ef_construction_values;  // Construction ef values
  
  // Test related
  int k = 100;                        // top-k value
  std::vector<int> ef_search_values;  // Search ef values
  std::vector<int> num_threads_values; // Number of threads
  
  // Google Benchmark parameters
  std::string benchmark_filter;       // --benchmark_filter
  std::string benchmark_format;       // --benchmark_format
  std::string benchmark_out;          // --benchmark_out
  
  // Runtime mode
  bool build_index_parallel = false;  // Build indexes in parallel
};

// Command line parser
class CliParser {
 public:
  // Parse command line arguments
  static CliConfig Parse(int argc, char** argv);
  
  // Print help message
  static void PrintHelp(const char* program_name);
  
  // Convert index type to string
  static std::string IndexTypeToString(IndexType type);
  
  // Parse index type from string
  static IndexType ParseIndexType(const std::string& str);
  
 private:
  // Parse comma-separated integer values
  static std::vector<int> ParseIntValues(const std::string& str);
  
  // Parse comma-separated index types
  static std::vector<IndexType> ParseIndexTypes(const std::string& str);
};

}  // namespace naive
