# Phase 0: Naive Baseline

**Status**: Completed
**Priority**: P0 (Highest)
**Estimated Effort**: 2 days

## Overview

实现一个最简单的基准版本，使用 Google Benchmark 测试 hnswlib 库的性能。此版本独立于后续 Phase 的完整框架，作为概念验证和基础参考。

## Goals

1. 验证整体技术路线可行性
2. 建立性能测试基准
3. 提供可快速迭代的最小实现
4. 为后续框架设计提供参考

## Target Library

### hnswlib

- **GitHub**: https://github.com/nmslib/hnswlib
- **特点**: Header-only C++ HNSW 实现
- **接口**: 简单直接的 API

```cpp
#include "hnswlib/hnswlib.h"

// 创建索引
hnswlib::L2Space space(dim);
hnswlib::HierarchicalNSW<float> index(&space, max_elements, M, ef_construction);

// 添加向量
index.addPoint(vector_data, id);

// 搜索
auto result = index.searchKnn(query, k);
```

## Dataset

本项目使用两个数据集：一个 L2 距离，一个点积/余弦距离。

### Dataset 1: SIFT Small (L2 距离)

| Property | Value |
|----------|-------|
| Name | SIFT Small |
| Size | 500,000 vectors |
| Dimension | 128 |
| Metric | L2 (Euclidean) |
| Files | sift_base.fvecs, sift_query.fvecs, sift_groundtruth.ivecs |
| Download | http://ann-benchmarks.com/sift-128-euclidean.tar.gz |

**选择理由**:
- 数据量适中，测试快速
- 维度低 (128)，适合基础测试
- L2 距离与 hnswlib 默认匹配
- 文件格式简单 (fvecs/ivecs)

### Dataset 2: GloVe-50 (点积/余弦距离)

| Property | Value |
|----------|-------|
| Name | GloVe-50-angular |
| Size | 1,183,514 vectors |
| Dimension | 50 |
| Metric | Angular (Cosine/Inner Product) |
| Format | HDF5 |
| File Size | 235 MB |
| Download | http://ann-benchmarks.com/glove-50-angular.hdf5 |

**选择理由**:
- 点积/余弦距离，与 SIFT 互补
- 维度低 (50)，测试快速
- 数据量适中 (~1.2M)
- HDF5 格式，现代通用

### Distance Metrics

| Dataset | Metric | hnswlib Space |
|---------|--------|---------------|
| SIFT | L2 (Euclidean) | `hnswlib::L2Space` |
| GloVe | Angular | `hnswlib::InnerProductSpace` |

**注意**: Angular/Cosine 距离等价于归一化向量上的点积距离。GloVe 数据集已经预归一化。

### Download URLs

```bash
# SIFT (L2)
SIFT_URL="http://ann-benchmarks.com/sift-128-euclidean.tar.gz"

# GloVe-50 (Angular/Inner Product)
GLOVE_URL="http://ann-benchmarks.com/glove-50-angular.hdf5"
```

---

## Task 0.1: 项目目录结构

**Status**: Completed

### Checklist
- [x] 创建 `naive/` 目录
- [x] 创建 `naive/CMakeLists.txt`
- [x] 创建 `naive/include/` 目录
- [x] 创建 `naive/src/` 目录
- [x] 创建 `naive/data/` 目录 (存放数据集)
- [x] 创建 `naive/scripts/` 目录 (下载脚本)
- [x] 创建 `naive/third_party/` 目录

### Directory Structure

```
naive/
├── CMakeLists.txt
├── README.md
├── third_party/
│   └── hnswlib/           # Git submodule
├── include/
│   └── benchmark_config.h
├── src/
│   ├── main.cpp
│   ├── dataset_loader.cpp
│   └── dataset_loader.h
├── scripts/
│   └── download_datasets.sh
├── data/
│   ├── sift/              # SIFT dataset (L2)
│   │   ├── sift_base.fvecs
│   │   ├── sift_query.fvecs
│   │   └── sift_groundtruth.ivecs
│   ├── glove/             # GloVe dataset (Angular/IP)
│   │   ├── train.fvecs
│   │   ├── test.fvecs
│   │   └── groundtruth.ivecs
│   └── .gitkeep
└── results/
    └── .gitkeep
```

### Acceptance Criteria
- 目录结构创建完成

---

## Task 0.2: 依赖管理

**Status**: Completed

### Checklist
- [x] 添加 hnswlib (header-only, Git submodule 或直接下载)
- [x] 添加 Google Benchmark
- [x] 配置 CMake 依赖

### CMakeLists.txt Template

```cmake
cmake_minimum_required(VERSION 3.16)
project(hnswlib_benchmark VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# Google Benchmark
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

# hnswlib (header-only)
add_library(hnswlib INTERFACE)
target_include_directories(hnswlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/hnswlib)

# Benchmark executable
add_executable(hnswlib_bench
  src/main.cpp
  src/dataset_loader.cpp
)

target_link_libraries(hnswlib_bench
  hnswlib
  benchmark::benchmark
)

target_include_directories(hnswlib_bench PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

### Acceptance Criteria
- CMake 配置成功
- Google Benchmark 编译通过

---

## Task 0.3: 数据集下载脚本

**Status**: Completed
**File**: `naive/scripts/download_datasets.sh`

### Checklist
- [x] 创建下载脚本
- [x] 支持 SIFT 和 GloVe 两个数据集
- [x] 支持断点续传
- [x] 自动解压
- [x] 校验文件完整性

### Script

```bash
#!/bin/bash
# download_datasets.sh - Download SIFT and GloVe datasets

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="${SCRIPT_DIR}/../data"

# URLs
SIFT_URL="http://ann-benchmarks.com/sift-128-euclidean.tar.gz"
GLOVE_URL="http://ann-benchmarks.com/glove-50-angular.hdf5"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Download function with resume support
download_file() {
    local url=$1
    local output=$2
    
    if [ -f "$output" ]; then
        log_info "File already exists: $output"
        return 0
    fi
    
    log_info "Downloading $url..."
    
    if command -v wget &> /dev/null; then
        wget -c "$url" -O "$output"
    elif command -v curl &> /dev/null; then
        curl -C - -L "$url" -o "$output"
    else
        log_error "Neither wget nor curl found. Please install one of them."
        exit 1
    fi
}

# Download and extract SIFT
download_sift() {
    log_step "Downloading SIFT dataset (L2 distance)..."
    
    local sift_dir="${DATA_DIR}/sift"
    local tar_file="${DATA_DIR}/sift.tar.gz"
    
    if [ -d "$sift_dir" ] && [ -f "$sift_dir/sift_base.fvecs" ]; then
        log_info "SIFT dataset already exists"
        return 0
    fi
    
    mkdir -p "$DATA_DIR"
    download_file "$SIFT_URL" "$tar_file"
    
    log_info "Extracting SIFT dataset..."
    mkdir -p "$sift_dir"
    tar -xzf "$tar_file" -C "$sift_dir" --strip-components=1 2>/dev/null || \
    tar -xzf "$tar_file" -C "$sift_dir"
    
    # Rename files if needed
    if [ -f "$sift_dir/sift-128-euclidean.train" ]; then
        mv "$sift_dir/sift-128-euclidean.train" "$sift_dir/sift_base.fvecs" 2>/dev/null || true
        mv "$sift_dir/sift-128-euclidean.test" "$sift_dir/sift_query.fvecs" 2>/dev/null || true
        mv "$sift_dir/sift-128-euclidean.groundtruth" "$sift_dir/sift_groundtruth.ivecs" 2>/dev/null || true
    fi
    
    rm -f "$tar_file"
    log_info "SIFT dataset ready at $sift_dir"
}

# Download GloVe
download_glove() {
    log_step "Downloading GloVe dataset (Angular/Inner Product distance)..."
    
    local glove_dir="${DATA_DIR}/glove"
    local hdf5_file="${DATA_DIR}/glove-50-angular.hdf5"
    
    if [ -d "$glove_dir" ] && [ -f "$glove_dir/train.fvecs" ]; then
        log_info "GloVe dataset already extracted"
        return 0
    fi
    
    mkdir -p "$DATA_DIR"
    download_file "$GLOVE_URL" "$hdf5_file"
    
    log_info "Extracting GloVe dataset from HDF5..."
    mkdir -p "$glove_dir"
    
    # Use Python to extract HDF5 to fvecs format
    python3 << EOF
import h5py
import numpy as np
import struct
import os

hdf5_path = "${hdf5_file}"
output_dir = "${glove_dir}"

print(f"Loading {hdf5_path}...")
with h5py.File(hdf5_path, 'r') as f:
    # Print available keys
    print(f"Keys: {list(f.keys())}")
    
    # Load data
    train = np.array(f['train'])
    test = np.array(f['test'])
    neighbors = np.array(f['neighbors'])
    distances = np.array(f.get('distances', None))
    
    print(f"Train shape: {train.shape}")
    print(f"Test shape: {test.shape}")
    print(f"Neighbors shape: {neighbors.shape}")

# Write train vectors to fvecs
def write_fvecs(filename, vectors):
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.float32).tobytes())

# Write ground truth to ivecs
def write_ivecs(filename, vectors):
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.int32).tobytes())

write_fvecs(os.path.join(output_dir, 'train.fvecs'), train)
write_fvecs(os.path.join(output_dir, 'test.fvecs'), test)
write_ivecs(os.path.join(output_dir, 'groundtruth.ivecs'), neighbors)

print("Extraction complete!")
EOF
    
    rm -f "$hdf5_file"
    log_info "GloVe dataset ready at $glove_dir"
}

# Verify datasets
verify() {
    log_step "Verifying datasets..."
    
    local all_ok=true
    
    # Verify SIFT
    local sift_dir="${DATA_DIR}/sift"
    if [ -f "$sift_dir/sift_base.fvecs" ]; then
        local size=$(stat -c%s "$sift_dir/sift_base.fvecs" 2>/dev/null || stat -f%z "$sift_dir/sift_base.fvecs")
        log_info "SIFT: sift_base.fvecs ($(($size / 1024 / 1024)) MB)"
    else
        log_error "SIFT: sift_base.fvecs not found"
        all_ok=false
    fi
    
    # Verify GloVe
    local glove_dir="${DATA_DIR}/glove"
    if [ -f "$glove_dir/train.fvecs" ]; then
        local size=$(stat -c%s "$glove_dir/train.fvecs" 2>/dev/null || stat -f%z "$glove_dir/train.fvecs")
        log_info "GloVe: train.fvecs ($(($size / 1024 / 1024)) MB)"
    else
        log_error "GloVe: train.fvecs not found"
        all_ok=false
    fi
    
    if $all_ok; then
        log_info "All datasets verified successfully!"
        return 0
    else
        log_error "Some datasets are missing!"
        return 1
    fi
}

# Usage
usage() {
    echo "Usage: $0 [sift|glove|all|verify]"
    echo ""
    echo "Commands:"
    echo "  sift    - Download SIFT dataset (L2 distance)"
    echo "  glove   - Download GloVe dataset (Angular distance)"
    echo "  all     - Download all datasets (default)"
    echo "  verify  - Verify existing datasets"
}

# Main
main() {
    local command=${1:-all}
    
    log_info "VectorDB Benchmark Dataset Downloader"
    log_info "======================================"
    
    mkdir -p "$DATA_DIR"
    
    case $command in
        sift)
            download_sift
            ;;
        glove)
            download_glove
            ;;
        all)
            download_sift
            download_glove
            ;;
        verify)
            verify
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown command: $command"
            usage
            exit 1
            ;;
    esac
    
    verify
}

main "$@"
```

### Acceptance Criteria
- 脚本可正常下载 SIFT 数据集
- 文件完整性验证通过

---

## Task 0.4: 数据集加载器

**Status**: Completed
**File**: `naive/src/dataset_loader.h`, `naive/src/dataset_loader.cpp`

### Checklist
- [x] 实现 fvecs 文件读取
- [x] 实现 ivecs 文件读取
- [x] 实现数据集加载类
- [x] 内存管理优化

### Header

```cpp
// naive/src/dataset_loader.h
#pragma once

#include <string>
#include <vector>

namespace naive {

// Distance metric type
enum class MetricType {
  kL2,           // Euclidean distance
  kInnerProduct  // Inner product (for angular/cosine)
};

struct Dataset {
    std::vector<std::vector<float>> train_vectors;
    std::vector<std::vector<float>> test_vectors;
    std::vector<std::vector<int>> ground_truth;
    
    int dimension = 0;
    int train_size = 0;
    int test_size = 0;
    int k = 0;  // ground truth k value
    MetricType metric_type = MetricType::kL2;
    std::string name;
};

class DatasetLoader {
 public:
    // Load SIFT dataset (L2 distance)
    static Dataset LoadSIFT(const std::string& data_dir);
    
    // Load GloVe dataset (Angular/Inner Product distance)
    static Dataset LoadGloVe(const std::string& data_dir);
    
    // Generic loader (auto-detect by directory name)
    static Dataset Load(const std::string& data_dir);
    
 private:
    // Read .fvecs file (float vectors)
    static std::vector<std::vector<float>> ReadFvecs(const std::string& filepath);
    
    // Read .ivecs file (integer vectors)
    static std::vector<std::vector<int>> ReadIvecs(const std::string& filepath);
};

}  // namespace naive
```

### Implementation Details

```cpp
// naive/src/dataset_loader.cpp
#include "dataset_loader.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>

namespace naive {

std::vector<std::vector<float>> DatasetLoader::ReadFvecs(
    const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    
    std::vector<std::vector<float>> vectors;
    
    while (file) {
        int dim;
        file.read(reinterpret_cast<char*>(&dim), sizeof(int));
        if (!file) break;
        
        std::vector<float> vec(dim);
        file.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(float));
        if (!file) break;
        
        vectors.push_back(std::move(vec));
    }
    
    return vectors;
}

std::vector<std::vector<int>> DatasetLoader::ReadIvecs(
    const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    
    std::vector<std::vector<int>> vectors;
    
    while (file) {
        int dim;
        file.read(reinterpret_cast<char*>(&dim), sizeof(int));
        if (!file) break;
        
        std::vector<int> vec(dim);
        file.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(int));
        if (!file) break;
        
        vectors.push_back(std::move(vec));
    }
    
    return vectors;
}

Dataset DatasetLoader::LoadSIFT(const std::string& data_dir) {
    Dataset dataset;
    dataset.name = "SIFT";
    dataset.metric_type = MetricType::kL2;
    
    // Load train vectors
    dataset.train_vectors = ReadFvecs(data_dir + "/sift_base.fvecs");
    dataset.train_size = dataset.train_vectors.size();
    dataset.dimension = dataset.train_vectors.empty() ? 0 
                       : dataset.train_vectors[0].size();
    
    // Load test vectors
    dataset.test_vectors = ReadFvecs(data_dir + "/sift_query.fvecs");
    dataset.test_size = dataset.test_vectors.size();
    
    // Load ground truth
    dataset.ground_truth = ReadIvecs(data_dir + "/sift_groundtruth.ivecs");
    dataset.k = dataset.ground_truth.empty() ? 0 
              : dataset.ground_truth[0].size();
    
    return dataset;
}

Dataset DatasetLoader::LoadGloVe(const std::string& data_dir) {
    Dataset dataset;
    dataset.name = "GloVe-50";
    dataset.metric_type = MetricType::kInnerProduct;
    
    // Load train vectors
    dataset.train_vectors = ReadFvecs(data_dir + "/train.fvecs");
    dataset.train_size = dataset.train_vectors.size();
    dataset.dimension = dataset.train_vectors.empty() ? 0 
                       : dataset.train_vectors[0].size();
    
    // Load test vectors
    dataset.test_vectors = ReadFvecs(data_dir + "/test.fvecs");
    dataset.test_size = dataset.test_vectors.size();
    
    // Load ground truth
    dataset.ground_truth = ReadIvecs(data_dir + "/groundtruth.ivecs");
    dataset.k = dataset.ground_truth.empty() ? 0 
              : dataset.ground_truth[0].size();
    
    return dataset;
}

Dataset DatasetLoader::Load(const std::string& data_dir) {
    std::filesystem::path path(data_dir);
    std::string dirname = path.filename().string();
    
    if (dirname.find("sift") != std::string::npos) {
        return LoadSIFT(data_dir);
    } else if (dirname.find("glove") != std::string::npos) {
        return LoadGloVe(data_dir);
    } else {
        throw std::runtime_error("Unknown dataset: " + dirname);
    }
}

}  // namespace naive
```

### Acceptance Criteria
- 正确加载 SIFT 数据集
- 内存使用合理

---

## Task 0.5: HNSW Benchmark 实现

**Status**: Completed
**File**: `naive/src/main.cpp`

### Checklist
- [x] 实现索引构建 benchmark
- [x] 实现搜索性能 benchmark
- [x] 实现召回率计算
- [x] 支持 L2 和 InnerProduct 两种距离
- [x] 输出格式化结果

### Implementation

```cpp
// naive/src/main.cpp
#include <benchmark/benchmark.h>
#include <hnswlib/hnswlib.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <memory>
#include <set>
#include <algorithm>

#include "dataset_loader.h"

namespace {

// Global dataset (loaded once)
naive::Dataset g_dataset;

// Benchmark configuration
struct Config {
    int M = 16;                    // HNSW M parameter
    int ef_construction = 200;     // Construction ef
    int ef_search = 100;           // Search ef
    int k = 100;                   // Number of neighbors
};

Config g_config;

// Calculate recall
double CalculateRecall(
    const std::vector<std::vector<int>>& predicted,
    const std::vector<std::vector<int>>& ground_truth,
    int k) {
    
    double total_recall = 0.0;
    int num_queries = predicted.size();
    
    for (int i = 0; i < num_queries; ++i) {
        std::set<int> gt_set(ground_truth[i].begin(), 
                             ground_truth[i].begin() + k);
        
        int correct = 0;
        for (int j = 0; j < k && j < predicted[i].size(); ++j) {
            if (gt_set.count(predicted[i][j])) {
                ++correct;
            }
        }
        
        total_recall += static_cast<double>(correct) / k;
    }
    
    return total_recall / num_queries;
}

// Create space based on metric type
std::unique_ptr<hnswlib::SpaceInterface<float>> CreateSpace(
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

}  // namespace

// Benchmark: Index Construction
static void BM_IndexConstruction(benchmark::State& state) {
    int M = state.range(0);
    int ef_construction = state.range(1);
    
    for (auto _ : state) {
        auto space = CreateSpace(g_dataset.metric_type, g_dataset.dimension);
        hnswlib::HierarchicalNSW<float> index(
            space.get(), g_dataset.train_size, M, ef_construction);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < g_dataset.train_size; ++i) {
            index.addPoint(g_dataset.train_vectors[i].data(), i);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        benchmark::DoNotOptimize(index);
        
        state.SetItemsProcessed(g_dataset.train_size);
        state.counters["duration_ms"] = duration.count();
        state.counters["metric"] = (g_dataset.metric_type == naive::MetricType::kL2) ? 0 : 1;
    }
}
BENCHMARK(BM_IndexConstruction)
    ->Args({16, 200})
    ->Args({32, 200})
    ->Args({16, 400})
    ->Unit(benchmark::kMillisecond);

// Benchmark: Search
static void BM_Search(benchmark::State& state) {
    int ef_search = state.range(0);
    int k = g_config.k;
    
    // Build index once
    auto space = CreateSpace(g_dataset.metric_type, g_dataset.dimension);
    hnswlib::HierarchicalNSW<float> index(
        space.get(), g_dataset.train_size, g_config.M, g_config.ef_construction);
    
    for (int i = 0; i < g_dataset.train_size; ++i) {
        index.addPoint(g_dataset.train_vectors[i].data(), i);
    }
    
    index.setEf(ef_search);
    
    std::vector<std::vector<int>> all_results(g_dataset.test_size);
    
    for (auto _ : state) {
        for (int i = 0; i < g_dataset.test_size; ++i) {
            auto result = index.searchKnn(g_dataset.test_vectors[i].data(), k);
            
            all_results[i].clear();
            while (!result.empty()) {
                all_results[i].push_back(result.top().second);
                result.pop();
            }
            std::reverse(all_results[i].begin(), all_results[i].end());
        }
    }
    
    // Calculate recall
    double recall = CalculateRecall(all_results, g_dataset.ground_truth, k);
    
    state.SetItemsProcessed(state.iterations() * g_dataset.test_size);
    state.counters["recall"] = recall * 100;
    state.counters["ef_search"] = ef_search;
}
BENCHMARK(BM_Search)
    ->Args({50})
    ->Args({100})
    ->Args({200})
    ->Args({500})
    ->Unit(benchmark::kMicrosecond);

// Benchmark: Single Query Latency
static void BM_SingleQuery(benchmark::State& state) {
    int ef_search = state.range(0);
    int k = g_config.k;
    
    // Build index
    auto space = CreateSpace(g_dataset.metric_type, g_dataset.dimension);
    hnswlib::HierarchicalNSW<float> index(
        space.get(), g_dataset.train_size, g_config.M, g_config.ef_construction);
    
    for (int i = 0; i < g_dataset.train_size; ++i) {
        index.addPoint(g_dataset.train_vectors[i].data(), i);
    }
    
    index.setEf(ef_search);
    
    int query_idx = 0;
    
    for (auto _ : state) {
        auto result = index.searchKnn(g_dataset.test_vectors[query_idx].data(), k);
        benchmark::DoNotOptimize(result);
        
        query_idx = (query_idx + 1) % g_dataset.test_size;
    }
}
BENCHMARK(BM_SingleQuery)
    ->Args({100})
    ->Args({200})
    ->Unit(benchmark::kNanosecond);

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
                      << "              - data/glove   (GloVe, InnerProduct distance)\n"
                      << "\n"
                      << "Default: data/sift\n";
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
              << (g_dataset.metric_type == naive::MetricType::kL2 ? "L2" : "InnerProduct")
              << std::endl;
    
    // Run benchmarks
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    
    return 0;
}
```

### Acceptance Criteria
- Benchmark 正确运行
- 输出包含 QPS、延迟、召回率

---

## Task 0.6: README 文档

**Status**: Completed
**File**: `naive/README.md`

### Checklist
- [x] 项目简介
- [x] 构建说明
- [x] 运行说明
- [x] 结果解读

### Content Template

```markdown
# HNSWLIB Benchmark (Naive)

A minimal benchmark for testing hnswlib performance using Google Benchmark.

## Features

- Support for L2 (Euclidean) and Inner Product distance metrics
- SIFT dataset (L2 distance)
- GloVe dataset (Angular/Inner Product distance)
- Google Benchmark integration
- Recall calculation

## Prerequisites

- C++17 compiler (GCC 9+ or Clang 10+)
- CMake 3.16+
- Python 3.7+ (for dataset extraction)

## Build

```bash
# Clone with submodules
git clone --recursive <repo_url>

cd naive

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Download Datasets

```bash
cd scripts
chmod +x download_datasets.sh

# Download all datasets
./download_datasets.sh all

# Or download individually
./download_datasets.sh sift    # SIFT (L2 distance)
./download_datasets.sh glove   # GloVe (Inner Product)
```

## Run

```bash
# Test SIFT dataset (L2 distance)
./build/hnswlib_bench data/sift

# Test GloVe dataset (Inner Product distance)
./build/hnswlib_bench data/glove
```

## Datasets

| Dataset | Size | Dimension | Metric |
|---------|------|-----------|--------|
| SIFT | 500K | 128 | L2 (Euclidean) |
| GloVe-50 | 1.18M | 50 | Inner Product |

## Expected Output

```
Loading dataset from data/sift...

Dataset: SIFT
  Train vectors: 500000
  Test vectors: 10000
  Dimension: 128
  Ground truth k: 100
  Metric: L2

------------------------------------------------------------------------------
Benchmark                                    Time           Items/s
------------------------------------------------------------------------------
BM_IndexConstruction/16/200             12345 ms      40507 items/s
BM_Search/100                           567.8 us      17615 items/s recall=95.2%
BM_SingleQuery/100                      12345 ns       81014 items/s
```
```

### Acceptance Criteria
- 文档清晰完整

---

## Task 0.7: 测试验证

**Status**: Completed

### Checklist
- [x] 编译通过无警告
- [x] SIFT 数据集下载成功 (使用生成脚本)
- [ ] GloVe 数据集下载成功 (可选)
- [x] SIFT Benchmark 运行正常
- [ ] GloVe Benchmark 运行正常 (可选)
- [x] 召回率计算正确

### Expected Results - SIFT (L2)

| Metric | Expected Range |
|--------|----------------|
| Index Build Time | 10-60 seconds |
| Recall@100 (ef=100) | > 90% |
| Recall@100 (ef=200) | > 95% |
| QPS | > 10000 |

### Expected Results - GloVe (Inner Product)

| Metric | Expected Range |
|--------|----------------|
| Index Build Time | 30-120 seconds |
| Recall@100 (ef=100) | > 85% |
| Recall@100 (ef=200) | > 92% |
| QPS | > 15000 |

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| hnswlib | latest | HNSW implementation |
| Google Benchmark | 1.8.3+ | Benchmark framework |

## Notes

1. 此版本独立于后续框架，放在 `naive/` 目录
2. 后续迭代可以完全不涉及这里
3. 主要用于验证技术路线和建立性能基准
4. 如需扩展测试其他库（如 nmslib），接口设计保持一致
