# Phase 1: 核心框架重构

**Status**: Not Started
**Priority**: P0
**Estimated Effort**: 5 days
**Prerequisites**: Phase 0 (Completed)

## Overview

基于 Phase 0 的经验，重新设计模块化的核心框架：
1. **DataLoader 模块化** - 支持多种文件格式的独立适配器
2. **测试逻辑分离** - 每个三方库独立测试模块
3. **命令行解析独立** - main 函数保持简洁

## Architecture Design

### 目录结构

```
naive/
├── include/
│   ├── common/
│   │   ├── types.h              # 基础类型定义
│   │   ├── config.h             # 配置结构
│   │   └── metrics.h            # 性能指标
│   ├── loader/
│   │   ├── data_loader.h        # DataLoader 主接口
│   │   ├── format_adapter.h     # 格式适配器基类
│   │   ├── fvecs_adapter.h      # fvecs 格式适配器
│   │   ├── ivecs_adapter.h      # ivecs 格式适配器
│   │   └── hdf5_adapter.h       # hdf5 格式适配器
│   ├── benchmark/
│   │   ├── benchmark_base.h     # 基准测试基类
│   │   ├── hnswlib_bench.h      # hnswlib 测试
│   │   └── arena_hnswlib_bench.h # arena-hnswlib 测试
│   └── cli/
│       └── cli_parser.h         # 命令行解析
├── src/
│   ├── common/
│   │   ├── types.cpp
│   │   ├── config.cpp
│   │   └── metrics.cpp
│   ├── loader/
│   │   ├── data_loader.cpp
│   │   ├── fvecs_adapter.cpp
│   │   ├── ivecs_adapter.cpp
│   │   └── hdf5_adapter.cpp
│   ├── benchmark/
│   │   ├── benchmark_base.cpp
│   │   ├── hnswlib_bench.cpp
│   │   └── arena_hnswlib_bench.cpp
│   ├── cli/
│   │   └── cli_parser.cpp
│   └── main.cpp                  # 简洁的 main 函数
└── config/
    ├── benchmark_params.yaml     # 基准测试参数配置
    └── datasets.yaml             # 数据集配置
```

---

## Task 1.1: 基础类型定义

**Status**: Not Started
**File**: `include/common/types.h`

### Checklist
- [ ] 定义 `MetricType` 枚举 (kL2, kInnerProduct)
- [ ] 定义 `Dataset` 结构体
  - train_vectors, test_vectors
  - ground_truth
  - dimension, train_size, test_size, k
  - metric_type, name
- [ ] 定义 `SearchResult` 结构体
- [ ] 实现 `ToString()` 方法用于调试

### Acceptance Criteria
- 类型定义符合 Google C++ 规范
- 编译无警告

---

## Task 1.2: 配置系统

**Status**: Not Started
**File**: `include/common/config.h`

### Checklist
- [ ] 定义 `BenchmarkParams` 结构体
  - M, ef_construction, ef_search_values
  - k, num_threads_values
- [ ] 定义 `DatasetConfig` 结构体
  - path, name, metric_type
- [ ] 支持 YAML 配置文件读取
- [ ] 实现默认配置
- [ ] 实现配置验证

### YAML 配置示例

```yaml
# config/benchmark_params.yaml
benchmark:
  M_values: [16, 32]
  ef_construction: 200
  ef_search_values: [100, 200, 500, 1000]
  k: 10
  num_threads_values: [1, 2, 4, 8]

datasets:
  - name: "SIFT"
    path: "data/sift"
    metric: "L2"
  - name: "GloVe"
    path: "data/glove"
    metric: "InnerProduct"
```

### Dependencies
- yaml-cpp

---

## Task 1.3: DataLoader 模块 - 接口设计

**Status**: Not Started
**File**: `include/loader/format_adapter.h`

### 设计思路

```cpp
namespace naive {

// 格式适配器基类
class FormatAdapter {
public:
  virtual ~FormatAdapter() = default;
  
  // 检查文件是否匹配此格式
  virtual bool CanHandle(const std::string& filepath) const = 0;
  
  // 读取向量数据
  virtual std::vector<std::vector<float>> ReadVectors(
      const std::string& filepath) = 0;
  
  // 读取整数向量数据 (ground truth)
  virtual std::vector<std::vector<int>> ReadIntVectors(
      const std::string& filepath) = 0;
  
  // 获取格式名称
  virtual std::string GetFormatName() const = 0;
};

}  // namespace naive
```

### Checklist
- [ ] 定义 FormatAdapter 抽象基类
- [ ] 定义文件格式检测逻辑接口
- [ ] 定义向量读取接口
- [ ] 定义错误处理

---

## Task 1.4: DataLoader 模块 - fvecs 适配器

**Status**: Not Started
**File**: `include/loader/fvecs_adapter.h`, `src/loader/fvecs_adapter.cpp`

### 设计思路

```cpp
namespace naive {

class FvecsAdapter : public FormatAdapter {
public:
  bool CanHandle(const std::string& filepath) const override {
    return filepath.find(".fvecs") != std::string::npos;
  }
  
  std::vector<std::vector<float>> ReadVectors(
      const std::string& filepath) override;
  
  std::string GetFormatName() const override { return "fvecs"; }
};

}  // namespace naive
```

### fvecs 格式说明
- 每个向量：[dim(int32)] + [dim 个 float32]
- 读取流程：读取 dim → 读取 dim 个 float → 下一个向量

### Checklist
- [ ] 实现 CanHandle() 文件格式检测
- [ ] 实现 ReadVectors() 读取 fvecs
- [ ] 错误处理：文件不存在、格式错误
- [ ] 性能优化：预分配内存

---

## Task 1.5: DataLoader 模块 - ivecs 适配器

**Status**: Not Started
**File**: `include/loader/ivecs_adapter.h`, `src/loader/ivecs_adapter.cpp`

### 设计思路

```cpp
namespace naive {

class IvecsAdapter : public FormatAdapter {
public:
  bool CanHandle(const std::string& filepath) const override {
    return filepath.find(".ivecs") != std::string::npos;
  }
  
  std::vector<std::vector<int>> ReadIntVectors(
      const std::string& filepath) override;
  
  std::string GetFormatName() const override { return "ivecs"; }
};

}  // namespace naive
```

### ivecs 格式说明
- 每个向量：[dim(int32)] + [dim 个 int32]
- 用于存储 ground truth (最近邻索引)

### Checklist
- [ ] 实现 CanHandle() 文件格式检测
- [ ] 实现 ReadIntVectors() 读取 ivecs
- [ ] 错误处理

---

## Task 1.6: DataLoader 模块 - HDF5 适配器

**Status**: Not Started
**File**: `include/loader/hdf5_adapter.h`, `src/loader/hdf5_adapter.cpp`

### 设计思路

```cpp
namespace naive {

class Hdf5Adapter : public FormatAdapter {
public:
  bool CanHandle(const std::string& filepath) const override {
    return filepath.find(".hdf5") != std::string::npos;
  }
  
  std::vector<std::vector<float>> ReadVectors(
      const std::string& filepath) override;
  
  std::vector<std::vector<int>> ReadIntVectors(
      const std::string& filepath) override;
  
  // HDF5 特有：读取指定数据集
  std::vector<std::vector<float>> ReadDataset(
      const std::string& filepath, 
      const std::string& dataset_name);
  
  std::string GetFormatName() const override { return "hdf5"; }
};

}  // namespace naive
```

### HDF5 数据集映射
- train → train.fvecs
- test → test.fvecs  
- neighbors → groundtruth.ivecs

### Dependencies
- HighFive (Header-only HDF5 库)

### Checklist
- [ ] 添加 HighFive 依赖到 CMakeLists.txt
- [ ] 实现 CanHandle()
- [ ] 实现 ReadDataset() 读取 HDF5 数据集
- [ ] 实现 ReadVectors() 包装
- [ ] 实现数据集名称自动映射

---

## Task 1.7: DataLoader 模块 - 主接口

**Status**: Not Started
**File**: `include/loader/data_loader.h`, `src/loader/data_loader.cpp`

### 设计思路

```cpp
namespace naive {

class DataLoader {
public:
  DataLoader();
  
  // 注册格式适配器
  void RegisterAdapter(std::unique_ptr<FormatAdapter> adapter);
  
  // 加载数据集（自动检测格式）
  Dataset LoadDataset(const std::string& data_dir);
  
  // 根据配置加载
  Dataset LoadDataset(const DatasetConfig& config);
  
private:
  std::vector<std::unique_ptr<FormatAdapter>> adapters_;
  
  // 自动检测文件格式
  FormatAdapter* DetectFormat(const std::string& filepath);
  
  // 构建文件路径
  std::string BuildFilePath(const std::string& dir, 
                           const std::string& suffix);
};

// 工厂方法：创建默认的 DataLoader
std::unique_ptr<DataLoader> CreateDefaultDataLoader();

}  // namespace naive
```

### Checklist
- [ ] 实现 DataLoader 类
- [ ] 实现 RegisterAdapter() 注册机制
- [ ] 实现 DetectFormat() 自动检测
- [ ] 实现 LoadDataset() 加载逻辑
- [ ] 实现 CreateDefaultDataLoader() 工厂方法
- [ ] 注册 fvecs, ivecs, hdf5 适配器

---

## Task 1.8: Benchmark 模块 - 基类设计

**Status**: Not Started
**File**: `include/benchmark/benchmark_base.h`, `src/benchmark/benchmark_base.cpp`

### 设计思路

```cpp
namespace naive {

// 基准测试结果
struct BenchmarkResult {
  std::string library_name;
  std::string operation;  // "IndexConstruction", "Search", "ConcurrentSearch"
  std::map<std::string, double> metrics;  // recall, qps, latency_ms
  std::map<std::string, int> params;      // M, ef_search, threads
};

// 基准测试基类
class BenchmarkBase {
public:
  explicit BenchmarkBase(const std::string& name) : name_(name) {}
  virtual ~BenchmarkBase() = default;
  
  // 初始化索引
  virtual void Init(const Dataset& dataset, 
                   const BenchmarkParams& params) = 0;
  
  // 构建索引
  virtual void BuildIndex(int M, int ef_construction) = 0;
  
  // 搜索
  virtual std::vector<std::vector<int>> Search(
      const std::vector<std::vector<float>>& queries,
      int k, int ef_search) = 0;
  
  // 并发搜索
  virtual std::vector<std::vector<int>> ConcurrentSearch(
      const std::vector<std::vector<float>>& queries,
      int k, int ef_search, int num_threads) = 0;
  
  // 获取库名称
  std::string GetName() const { return name_; }
  
protected:
  std::string name_;
  Dataset dataset_;
};

}  // namespace naive
```

### Checklist
- [ ] 定义 BenchmarkResult 结构
- [ ] 定义 BenchmarkBase 抽象类
- [ ] 定义生命周期接口 (Init, BuildIndex, Search)
- [ ] 定义结果收集接口

---

## Task 1.9: Benchmark 模块 - hnswlib 实现

**Status**: Not Started
**File**: `include/benchmark/hnswlib_bench.h`, `src/benchmark/hnswlib_bench.cpp`

### 设计思路

```cpp
namespace naive {

class HnswlibBenchmark : public BenchmarkBase {
public:
  HnswlibBenchmark();
  ~HnswlibBenchmark() override;
  
  void Init(const Dataset& dataset, 
           const BenchmarkParams& params) override;
  
  void BuildIndex(int M, int ef_construction) override;
  
  std::vector<std::vector<int>> Search(
      const std::vector<std::vector<float>>& queries,
      int k, int ef_search) override;
  
  std::vector<std::vector<int>> ConcurrentSearch(
      const std::vector<std::vector<float>>& queries,
      int k, int ef_search, int num_threads) override;
  
private:
  std::unique_ptr<hnswlib::SpaceInterface<float>> space_;
  std::unique_ptr<hnswlib::HierarchicalNSW<float>> index_;
};

}  // namespace naive
```

### Checklist
- [ ] 实现 HnswlibBenchmark 类
- [ ] 实现 Init() 初始化空间
- [ ] 实现 BuildIndex() 构建索引
- [ ] 实现 Search() 单线程搜索
- [ ] 实现 ConcurrentSearch() 多线程搜索

---

## Task 1.10: Benchmark 模块 - arena-hnswlib 实现

**Status**: Not Started
**File**: `include/benchmark/arena_hnswlib_bench.h`, `src/benchmark/arena_hnswlib_bench.cpp`

### Checklist
- [ ] 实现 ArenaHnswlibBenchmark 类
- [ ] 实现 L2 和 InnerProduct 空间初始化
- [ ] 实现构建和搜索接口
- [ ] 实现并发搜索

---

## Task 1.11: CLI 模块

**Status**: Not Started
**File**: `include/cli/cli_parser.h`, `src/cli/cli_parser.cpp`

### 设计思路

```cpp
namespace naive {

struct CliOptions {
  std::string data_dir;
  std::string config_file;
  std::string output_format;  // "console", "json", "csv"
  bool verbose;
};

class CliParser {
public:
  CliParser();
  
  // 解析命令行参数
  CliOptions Parse(int argc, char** argv);
  
  // 打印帮助信息
  void PrintHelp(const std::string& program_name);
  
  // 打印版本信息
  void PrintVersion();
  
private:
  // 使用 CLI11 或简单手动解析
};

}  // namespace naive
```

### Checklist
- [ ] 定义 CliOptions 结构
- [ ] 实现 Parse() 参数解析
- [ ] 实现 PrintHelp() 帮助信息
- [ ] 支持 --config, --verbose, --output 选项
- [ ] 支持位置参数（数据集路径）

---

## Task 1.12: 重构 main.cpp

**Status**: Not Started
**File**: `src/main.cpp`

### 目标结构

```cpp
int main(int argc, char** argv) {
  // 1. 解析命令行
  auto options = naive::CliParser().Parse(argc, argv);
  
  // 2. 加载配置
  auto config = naive::ConfigLoader::Load(options.config_file);
  
  // 3. 创建 DataLoader 并加载数据
  auto loader = naive::CreateDefaultDataLoader();
  auto dataset = loader->LoadDataset(options.data_dir);
  
  // 4. 创建基准测试
  std::vector<std::unique_ptr<naive::BenchmarkBase>> benchmarks;
  benchmarks.push_back(std::make_unique<naive::HnswlibBenchmark>());
  benchmarks.push_back(std::make_unique<naive::ArenaHnswlibBenchmark>());
  
  // 5. 运行基准测试
  auto results = naive::RunBenchmarks(benchmarks, dataset, config);
  
  // 6. 输出结果
  naive::PrintResults(results, options.output_format);
  
  return 0;
}
```

### Checklist
- [ ] 重构 main.cpp 为简洁结构
- [ ] 使用各模块的接口
- [ ] 移除所有内联实现

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| yaml-cpp | >= 0.7 | Configuration parsing |
| HighFive | >= 2.7 | HDF5 reading |
| googletest | >= 1.11 | Unit testing |

## Notes

- Phase 1 完成后，项目将具备良好的模块化架构
- 新增文件格式只需实现新的 FormatAdapter
- 新增测试库只需实现新的 BenchmarkBase 子类
- 所有代码遵循 Google C++ 编码规范

---

## 扩展指南

### 新增测试库流程

```
1. 创建头文件 include/benchmark/mylib_bench.h
   └── 继承 BenchmarkBase，实现虚函数

2. 创建实现文件 src/benchmark/mylib_bench.cpp
   └── 实现具体测试逻辑

3. 更新 CMakeLists.txt
   └── 添加新源文件

4. 注册到 main.cpp
   └── benchmarks.push_back(std::make_unique<MyLibBenchmark>())
```

### 新增测试场景流程（当前架构）

```
1. 在 main.cpp 中添加 BM_* 函数
   └── 使用 BENCHMARK() 宏注册

2. 设置统一的 state.counters 字段
   └── recall, qps, latency_ms, duration_ms

3. 添加参数组合
   └── ->Args({param1, param2})
```

### 指标字段统一约定

为确保不同库结果可比，测试函数应统一输出：

| 操作类型 | 必需指标 |
|----------|----------|
| IndexConstruction | duration_ms, index_size_mb |
| Search | recall, qps, latency_ms |
| ConcurrentSearch | recall, qps, threads |
| SingleQuery | latency_us, recall |
