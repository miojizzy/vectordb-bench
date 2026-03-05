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

#include "cli_parser.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace naive {

std::string CliParser::IndexTypeToString(IndexType type) {
  switch (type) {
    case IndexType::kHnswlib:
      return "hnswlib";
    case IndexType::kArenaDoubleM:
      return "arena-DoubleM";
    case IndexType::kArenaHeuristicOnly:
      return "arena-HeuristicOnly";
    case IndexType::kArenaHeuristicPlusClosest:
      return "arena-HeuristicPlusClosest";
    default:
      return "Unknown";
  }
}

IndexType CliParser::ParseIndexType(const std::string& str) {
  std::string lower = str;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  
  if (lower == "hnswlib" || lower == "baseline") {
    return IndexType::kHnswlib;
  } else if (lower == "doublem" || lower == "arena_doublem") {
    return IndexType::kArenaDoubleM;
  } else if (lower == "heuristiconly" || lower == "arena_heuristiconly") {
    return IndexType::kArenaHeuristicOnly;
  } else if (lower == "heuristicplusclosest" || 
             lower == "arena_heuristicplusclosest") {
    return IndexType::kArenaHeuristicPlusClosest;
  } else {
    throw std::invalid_argument("Unknown index type: " + str);
  }
}

std::vector<int> CliParser::ParseIntValues(const std::string& str) {
  std::vector<int> values;
  std::stringstream ss(str);
  std::string token;
  
  while (std::getline(ss, token, ',')) {
    try {
      values.push_back(std::stoi(token));
    } catch (const std::exception& e) {
      throw std::invalid_argument("Invalid integer value: " + token);
    }
  }
  
  return values;
}

std::vector<IndexType> CliParser::ParseIndexTypes(const std::string& str) {
  std::vector<IndexType> types;
  std::stringstream ss(str);
  std::string token;
  
  while (std::getline(ss, token, ',')) {
    // Trim whitespace
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);
    
    if (!token.empty()) {
      types.push_back(ParseIndexType(token));
    }
  }
  
  return types;
}

void CliParser::PrintHelp(const char* program_name) {
  std::cout << "Usage: " << program_name << " [options]\n\n";
  std::cout << "Options:\n";
  std::cout << "  --data_dir <path>              Dataset directory (required)\n";
  std::cout << "  --output_file <path>           Output file for results\n";
  std::cout << "  --index_types <types>          Index types (comma-separated)\n";
  std::cout << "                                Options: hnswlib, arena_doublem,\n";
  std::cout << "                                         arena_heuristiconly,\n";
  std::cout << "                                         arena_heuristicplusclosest\n";
  std::cout << "                                Default: hnswlib\n";
  std::cout << "  --M <values>                   HNSW M parameter (comma-separated)\n";
  std::cout << "                                Default: 16\n";
  std::cout << "  --ef_construction <values>     Construction ef (comma-separated)\n";
  std::cout << "                                Default: 200\n";
  std::cout << "  --ef_search <values>           Search ef (comma-separated)\n";
  std::cout << "                                Default: 100,200,500,1000\n";
  std::cout << "  --num_threads <values>         Thread counts (comma-separated)\n";
  std::cout << "                                Default: 1,2,4,8\n";
  std::cout << "  --k <value>                    Top-k value (default: 100)\n";
  std::cout << "  --build_parallel               Build indexes in parallel\n";
  std::cout << "  --benchmark_filter <pattern>   Google Benchmark filter\n";
  std::cout << "  --benchmark_format <format>    Google Benchmark output format\n";
  std::cout << "                                Options: console, json, csv\n";
  std::cout << "  --benchmark_out <file>         Google Benchmark output file\n";
  std::cout << "  --help                         Show this help message\n";
  std::cout << "\nExamples:\n";
  std::cout << "  # Run all tests\n";
  std::cout << "  " << program_name 
            << " --data_dir ./data/fashion-mnist-784-euclidean\n";
  std::cout << "\n";
  std::cout << "  # Run only search tests with parallel index building\n";
  std::cout << "  " << program_name 
            << " --data_dir ./data/fashion-mnist-784-euclidean \\\n";
  std::cout << "    --index_types hnswlib,arena_doublem \\\n";
  std::cout << "    --benchmark_filter=BM_Search --build_parallel\n";
  std::cout << "\n";
  std::cout << "  # Run only index build tests\n";
  std::cout << "  " << program_name 
            << " --data_dir ./data/fashion-mnist-784-euclidean \\\n";
  std::cout << "    --benchmark_filter=BM_IndexBuild\n";
}

CliConfig CliParser::Parse(int argc, char** argv) {
  CliConfig config;
  
  // Set defaults
  config.M_values = {16};
  config.ef_construction_values = {200};
  config.ef_search_values = {10, 20, 50, 100};
  config.num_threads_values = {1, 2, 4, 8};
  
  bool has_data_dir = false;
  
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    
    if (arg == "--help" || arg == "-h") {
      PrintHelp(argv[0]);
      exit(0);
    } else if (arg == "--data_dir") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--data_dir requires a value");
      }
      config.data_dir = argv[++i];
      has_data_dir = true;
    } else if (arg == "--output_file") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--output_file requires a value");
      }
      config.output_file = argv[++i];
    } else if (arg == "--index_types") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--index_types requires a value");
      }
      config.index_types = ParseIndexTypes(argv[++i]);
    } else if (arg == "--M") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--M requires a value");
      }
      config.M_values = ParseIntValues(argv[++i]);
    } else if (arg == "--ef_construction") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--ef_construction requires a value");
      }
      config.ef_construction_values = ParseIntValues(argv[++i]);
    } else if (arg == "--ef_search") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--ef_search requires a value");
      }
      config.ef_search_values = ParseIntValues(argv[++i]);
    } else if (arg == "--num_threads") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--num_threads requires a value");
      }
      config.num_threads_values = ParseIntValues(argv[++i]);
    } else if (arg == "--k") {
      if (i + 1 >= argc) {
        throw std::invalid_argument("--k requires a value");
      }
      try {
        config.k = std::stoi(argv[++i]);
      } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid k value");
      }
    } else if (arg == "--build_parallel") {
      config.build_index_parallel = true;
    } else if (arg.find("--benchmark_filter=") == 0) {
      config.benchmark_filter = arg.substr(strlen("--benchmark_filter="));
    } else if (arg.find("--benchmark_format=") == 0) {
      config.benchmark_format = arg.substr(strlen("--benchmark_format="));
    } else if (arg.find("--benchmark_out=") == 0) {
      config.benchmark_out = arg.substr(strlen("--benchmark_out="));
    } else if (arg[0] == '-') {
      // Unknown option
      throw std::invalid_argument("Unknown option: " + arg);
    }
    // Ignore other arguments (might be for Google Benchmark)
  }
  
  // Validate required arguments
  if (!has_data_dir) {
    throw std::invalid_argument("--data_dir is required");
  }
  
  // Set default index types if not specified
  if (config.index_types.empty()) {
    config.index_types = {IndexType::kHnswlib};
  }
  
  return config;
}

}  // namespace naive
