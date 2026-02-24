# Active Context

## Current Status

**Project Phase**: Phase 0 - Naive Baseline (COMPLETED)

**Next Action**: 开始实现 Phase 1 核心框架

## Phase 0 Completion Summary

### 实现内容

| Item | Status | Details |
|------|--------|---------|
| 项目目录结构 | Completed | `naive/` 目录完整 |
| 依赖管理 | Completed | hnswlib, arena-hnswlib, Google Benchmark |
| 数据集支持 | Completed | Fashion-MNIST, Last.fm, SIFT, GloVe |
| 数据集加载器 | Completed | fvecs/ivecs 格式, L2 + IP 距离 |
| Benchmark 实现 | Completed | hnswlib vs arena-hnswlib 对比 |
| 文档 | Completed | README.md |

### 项目结构

```
naive/
├── CMakeLists.txt
├── README.md
├── include/
│   └── benchmark_config.h
├── src/
│   ├── main.cpp
│   ├── dataset_loader.h
│   └── dataset_loader.cpp
├── scripts/
│   ├── download_datasets.sh
│   ├── convert_hdf5_to_fvecs.py
│   ├── generate_test_dataset.py
│   ├── generate_large_dataset.py
│   └── generate_ip_dataset.py
├── data/
│   ├── fashion-mnist/   # 60K train, 10K test, 784 dim, L2
│   ├── lastfm/          # 292K train, 50K test, 65 dim, IP
│   ├── sift/            # synthetic, 50K, 128 dim, L2
│   └── glove/           # synthetic, 50K, 128 dim, IP
├── results/
└── third_party/
    ├── hnswlib/         # Git submodule
    └── arena-hnswlib/   # Cloned
```

### Benchmark 结果

| 数据集 | 距离 | 库 | 索引构建 | 召回率@100 | QPS |
|--------|------|------|---------|-----------|-----|
| Fashion-MNIST | L2 | hnswlib | 22.1s | 99.3% | 4.0K/s |
| Fashion-MNIST | L2 | arena-hnswlib | 24.7s | 97.3% | 3.8K/s |
| Last.fm | IP | hnswlib | 47.3s | 96.0% | 13.7K/s |
| Last.fm | IP | arena-hnswlib | 64.9s | 90.7% | 9.5K/s |

**结论**: hnswlib 在两种距离度量下均优于 arena-hnswlib

## Recent Decisions

1. **数据集选择**: 使用 ann-benchmarks.com 的真实数据集 (Fashion-MNIST, Last.fm)
2. **对比库**: 添加 arena-hnswlib 进行性能对比
3. **技术路线**: Google Benchmark + fvecs/ivecs 格式验证可行
4. **下一步**: 开始 Phase 1 核心框架实现

## Work Focus

当前重点：准备开始 Phase 1 核心框架

### Phase 1 任务列表

| Task | 描述 |
|------|------|
| 1.1 | 创建项目目录结构 |
| 1.2 | 基础类型定义 (types.h) |
| 1.3 | 异常类定义 |
| 1.4 | 配置系统 (YAML) |
| 1.5 | 接口定义 - VectorDB |
| 1.6 | 接口定义 - Dataset |
| 1.7 | 接口定义 - BenchmarkCase |
| 1.8 | 日志系统 (spdlog) |
| 1.9 | CLI 工具 (CLI11) |
| 1.10 | 单元测试框架 |

## Technical Debt

暂无

## Open Questions

1. ~~是否需要支持其他 HNSW 库进行对比？~~ 已解决: 添加了 arena-hnswlib
2. Phase 1 是否需要支持分布式测试？
3. 数据集缓存策略如何设计？

## Dependencies Status

| Dependency | Status | Notes |
|------------|--------|-------|
| hnswlib | Integrated | Git submodule in naive/third_party/ |
| arena-hnswlib | Integrated | Cloned in naive/third_party/ |
| Google Benchmark | Integrated | CMake FetchContent |

## Phase 1 Technical Requirements

### 新依赖

| Library | Version | Purpose |
|---------|---------|---------|
| yaml-cpp | >= 0.7 | Configuration file parsing |
| nlohmann/json | >= 3.10 | JSON processing |
| CLI11 | >= 2.3 | Command line parsing |
| spdlog | >= 1.9 | Logging |
| googletest | >= 1.11 | Unit testing |

### 接口设计要点

```cpp
// VectorDB 接口
class VectorDB {
 public:
  virtual void Connect(const std::string& config) = 0;
  virtual void CreateCollection(const CollectionConfig& config) = 0;
  virtual void Insert(const std::vector<Vector>& vectors) = 0;
  virtual std::vector<SearchResult> Search(const Vector& query, int k) = 0;
  virtual void CreateIndex(const IndexConfig& config) = 0;
};

// Dataset 接口
class Dataset {
 public:
  virtual void Load(const std::string& path) = 0;
  virtual const std::vector<Vector>& GetTrainData() const = 0;
  virtual const std::vector<Vector>& GetTestData() const = 0;
  virtual const std::vector<std::vector<int>>& GetGroundTruth() const = 0;
};

// BenchmarkCase 接口
class BenchmarkCase {
 public:
  virtual void Setup(const BenchmarkConfig& config) = 0;
  virtual void Run() = 0;
  virtual BenchmarkResult GetResult() const = 0;
};
```

## Environment

- OS: Linux
- Compiler: GCC/Clang (C++17)
- Build: CMake 3.16+
