# Technical Context

## Technology Stack

### Language & Standard

| Item | Requirement |
|------|-------------|
| Language | C++17 |
| Compiler | GCC 9+, Clang 10+ |
| Build System | CMake 3.16+ |

### Core Dependencies (Phase 1+)

| Library | Version | Purpose |
|---------|---------|---------|
| yaml-cpp | >= 0.7 | Configuration file parsing |
| nlohmann/json | >= 3.10 | JSON processing |
| CLI11 | >= 2.3 | Command line parsing |
| spdlog | >= 1.9 | Logging |
| googletest | >= 1.11 | Unit testing |

### Phase 0 Dependencies (Implemented)

| Library | Version | Purpose | Integration |
|---------|---------|---------|-------------|
| hnswlib | latest | HNSW implementation (header-only) | Git submodule |
| arena-hnswlib | latest | Alternative HNSW with arena allocator | Git clone |
| Google Benchmark | >= 1.8.3 | Performance benchmarking | CMake FetchContent |

### Optional Database Dependencies

| Database | Dependency |
|----------|------------|
| Milvus | libmilvus (C++ SDK) |
| Qdrant | gRPC + Protobuf |
| PostgreSQL/PgVector | libpq |
| Elasticsearch | cpr (HTTP client) |
| Redis | hiredis |

## Directory Structure

```
vectordb_bench/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # Project documentation
├── LICENSE                     # MIT License
├── naive/                      # Phase 0: Naive Baseline (COMPLETED)
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── third_party/
│   │   ├── hnswlib/            # Git submodule
│   │   └── arena-hnswlib/      # Cloned
│   ├── include/
│   │   └── benchmark_config.h
│   ├── src/
│   │   ├── main.cpp            # Benchmark entry
│   │   ├── dataset_loader.h
│   │   └── dataset_loader.cpp
│   ├── scripts/
│   │   ├── download_datasets.sh
│   │   ├── convert_hdf5_to_fvecs.py
│   │   └── generate_*.py
│   ├── data/
│   │   ├── fashion-mnist/
│   │   ├── lastfm/
│   │   ├── sift/
│   │   └── glove/
│   └── results/
├── third_party/                # Third-party dependencies (Phase 1+)
│   └── CMakeLists.txt
├── include/                    # Header files (Phase 1+)
│   └── vectordb_bench/
│       ├── common/             # Common definitions
│       │   ├── types.h         # Basic type definitions
│       │   ├── config.h        # Configuration classes
│       │   ├── result.h        # Result structures
│       │   └── exception.h     # Exception definitions
│       ├── interface/          # Interface definitions
│       │   ├── vector_db.h     # Database interface
│       │   ├── dataset.h       # Dataset interface
│       │   └── benchmark_case.h # Benchmark case interface
│       ├── core/               # Core modules
│       │   ├── benchmark_runner.h
│       │   ├── task_scheduler.h
│       │   ├── metrics_collector.h
│       │   ├── cpu_affinity_manager.h
│       │   ├── thread_pool.h
│       │   └── comparison_runner.h
│       ├── dataset/            # Dataset processing
│       │   ├── dataset_loader.h
│       │   ├── sift_dataset.h
│       │   ├── gist_dataset.h
│       │   └── custom_dataset.h
│       ├── report/             # Report generation
│       │   ├── report_generator.h
│       │   ├── comparison_report.h
│       │   └── chart_builder.h
│       └── clients/            # Database clients
│           ├── milvus_client.h
│           ├── qdrant_client.h
│           └── ...
├── src/                        # Source files
│   ├── CMakeLists.txt
│   ├── common/
│   ├── interface/
│   ├── core/
│   ├── dataset/
│   ├── report/
│   └── clients/
├── tests/                      # Tests
│   ├── CMakeLists.txt
│   ├── unit/
│   └── integration/
├── config/                     # Configuration examples
│   └── examples/
└── tools/                      # Utility scripts
    └── generate_dataset.py
```

## Key Data Structures

### Phase 0 Data Format (Implemented)

```
fvecs file format (float vectors):
┌────────────┬─────────────────────────────┐
│ dim (int)  │ vector[dim] (float × dim)   │
│  4 bytes   │     dim × 4 bytes           │
├────────────┼─────────────────────────────┤
│ dim (int)  │ vector[dim] (float × dim)   │
│  4 bytes   │     dim × 4 bytes           │
└────────────┴─────────────────────────────┘

ivecs file format (integer vectors):
┌────────────┬─────────────────────────────┐
│ dim (int)  │ vector[dim] (int × dim)     │
│  4 bytes   │     dim × 4 bytes           │
└────────────┴─────────────────────────────┘
```

### Supported Datasets (Phase 0)

| Dataset | Size | Dimension | Metric | Source |
|---------|------|-----------|--------|--------|
| Fashion-MNIST | 60K train, 10K test | 784 | L2 | ann-benchmarks.com |
| Last.fm | 292K train, 50K test | 65 | Inner Product | ann-benchmarks.com |
| SIFT (synthetic) | 50K, 1K test | 128 | L2 | Generated |
| GloVe (synthetic) | 50K, 1K test | 128 | Inner Product | Generated |

### Types (common/types.h)

```cpp
namespace vectordb_bench {

using Vector = std::vector<float>;
using VectorId = int64_t;
using Distance = float;

enum class MetricType {
  kL2,
  kCosine,
  kInnerProduct,
  kHamming
};

struct SearchResult {
  VectorId id;
  Distance distance;
};

struct PerformanceMetrics {
  double qps;
  double latency_p50;
  double latency_p90;
  double latency_p99;
  double latency_avg;
  int64_t total_queries;
  std::chrono::microseconds total_time;
};

}  // namespace vectordb_bench
```

### Configuration (common/config.h)

```cpp
struct CpuConfig {
  int num_cores = -1;
  std::vector<int> core_ids;
  bool bind_threads = true;
  bool use_hyperthreading = true;
  int num_threads = -1;

  static CpuConfig SingleCore();
  static CpuConfig MultiCore(int n);
  static CpuConfig AllCores();
};

struct BenchmarkConfig {
  std::string db_label;
  int64_t k = 100;
  std::vector<int64_t> concurrency_levels;
  std::chrono::seconds concurrency_duration{30};
  std::chrono::seconds timeout{3600};
  CpuConfig cpu_config;
};

struct ComparisonConfig {
  std::string test_id;
  std::vector<DBConfig> databases;
  std::vector<CpuConfig> cpu_configs;
  std::string dataset_path;
  bool run_sequentially = true;
  bool share_dataset = true;
};
```

## Phase 0 Implementation Details

### DatasetLoader (naive/src/dataset_loader.cpp)

```cpp
class DatasetLoader {
 public:
  static Dataset LoadSIFT(const std::string& data_dir);
  static Dataset LoadGloVe(const std::string& data_dir);
  static Dataset LoadFashionMNIST(const std::string& data_dir);
  static Dataset LoadLastfm(const std::string& data_dir);
  static Dataset Load(const std::string& data_dir);  // Auto-detect

 private:
  static std::vector<std::vector<float>> ReadFvecs(const std::string& filepath);
  static std::vector<std::vector<int>> ReadIvecs(const std::string& filepath);
};
```

### Benchmark Structure (naive/src/main.cpp)

```cpp
// Benchmarks implemented:
// - BM_Hnswlib_IndexConstruction (M, ef_construction)
// - BM_Hnswlib_Search (ef_search)
// - BM_Hnswlib_SingleQuery (ef_search)
// - BM_ArenaHnswlib_IndexConstruction (M, ef_construction)
// - BM_ArenaHnswlib_Search (ef_search)
// - BM_ArenaHnswlib_SingleQuery (ef_search)

// Metrics collected:
// - Index build time (ms)
// - QPS (queries per second)
// - Recall@K
```

### CMakeLists.txt (naive/)

```cmake
cmake_minimum_required(VERSION 3.16)
project(hnswlib_bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# Dependencies
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

# hnswlib (header-only, submodule)
add_subdirectory(third_party/hnswlib)

# arena-hnswlib (header-only, cloned)
add_subdirectory(third_party/arena-hnswlib)

# Main executable
add_executable(hnswlib_bench
  src/main.cpp
  src/dataset_loader.cpp
)
target_link_libraries(hnswlib_bench benchmark::benchmark)
```

## CPU Affinity Implementation

### Linux Implementation

```cpp
bool SetLinuxAffinity(pid_t pid, const std::vector<int>& core_ids) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  
  for (int core_id : core_ids) {
    CPU_SET(core_id, &cpuset);
  }
  
  return sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset) == 0;
}
```

### Thread Pool with CPU Binding

```cpp
class ThreadPool {
 public:
  explicit ThreadPool(int num_threads, const std::vector<int>& core_ids = {});
  
  template<typename F, typename... Args>
  auto Submit(F&& f, Args&&... args) 
      -> std::future<typename std::invoke_result_t<F, Args...>>;
      
  void WaitAll();
};
```

## Scaling Efficiency Calculation

```cpp
double CalculateScalingEfficiency(double single_core_qps, 
                                  double multi_core_qps, 
                                  int num_cores) {
  double ideal_qps = single_core_qps * num_cores;
  return multi_core_qps / ideal_qps * 100.0;
}
```

## Build Configuration

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(vectordb_bench VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_MILVUS "Enable Milvus support" OFF)
option(ENABLE_QDRANT "Enable Qdrant support" OFF)

find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(src)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
```

## Coding Standards

Following Google C++ Style Guide with project-specific rules:

- File names: lowercase with underscores
- Class names: PascalCase
- Variable names: snake_case
- Constants: kCamelCase
- Namespace: lowercase based on project path
- Max line length: 80 characters
- Indentation: 2 spaces
- Use `#pragma once` for headers

## Testing Strategy

| Test Type | Framework | Coverage Target |
|-----------|-----------|-----------------|
| Unit Tests | GoogleTest | > 80% |
| Integration Tests | GoogleTest | Critical paths |
| Performance Tests | Custom | Benchmark validation |

## CI/CD Pipeline

```yaml
# .github/workflows/ci.yml
stages:
  - build
  - test
  - coverage

build:
  - cmake -B build
  - cmake --build build

test:
  - ctest --test-dir build

coverage:
  - gcov + lcov
```

## Phase 0 Key Findings

### Performance Comparison

| Metric | hnswlib | arena-hnswlib | Winner |
|--------|---------|---------------|--------|
| L2 Index Build | Faster | Slower | hnswlib |
| L2 Recall | Higher | Lower | hnswlib |
| IP Index Build | 37% faster | Slower | hnswlib |
| IP Recall | 5% higher | Lower | hnswlib |
| IP QPS | 44% higher | Lower | hnswlib |

### Technical Issues Found

1. **arena-hnswlib InnerProduct**: No SIMD optimization, uses pure loop
2. **arena-hnswlib L2**: Has AVX optimization, but still slower than hnswlib
3. **Data format**: fvecs/ivecs works well with HDF5 conversion

### Recommendations

1. Use **hnswlib** for both L2 and Inner Product distances
2. Use ann-benchmarks.com datasets for consistent benchmarking
3. HDF5 to fvecs conversion is reliable
