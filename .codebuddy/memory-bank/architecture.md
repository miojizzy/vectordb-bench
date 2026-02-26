# 架构设计概览

## 设计原则

1. **模块化** - 每个功能独立成模块，易于扩展
2. **接口抽象** - 通过基类定义接口，具体实现可替换
3. **配置驱动** - 参数配置与代码分离
4. **简洁 main** - main 函数只负责组装，不包含业务逻辑

---

## 模块架构

```
┌─────────────────────────────────────────────────────────────┐
│                         main.cpp                             │
│  (只负责组装各模块，不包含具体实现)                            │
└────────┬─────────────┬─────────────┬────────────────────────┘
         │             │             │
    ┌────▼────┐   ┌────▼────┐   ┌────▼────┐
    │   CLI   │   │ Config  │   │ Common  │
    │ Parser  │   │ Loader  │   │ Types   │
    └─────────┘   └─────────┘   └─────────┘
         │
         │
    ┌────▼─────────────────────────────────────────┐
    │              DataLoader 模块                  │
    │  ┌──────────────────────────────────────┐    │
    │  │         DataLoader (主接口)           │    │
    │  └───┬──────┬──────┬──────┬───────────┘    │
    │      │      │      │      │                 │
    │  ┌───▼──┐ ┌─▼───┐ ┌▼────┐ ┌▼────────────┐  │
    │  │fvecs │ │ivecs│ │hdf5 │ │future format│  │
    │  │适配器│ │适配器│ │适配器│ │  适配器     │  │
    │  └──────┘ └─────┘ └─────┘ └─────────────┘  │
    └──────────────────────────────────────────────┘
         │
         │ Dataset
         ▼
    ┌──────────────────────────────────────────────┐
    │              Benchmark 模块                   │
    │  ┌──────────────────────────────────────┐    │
    │  │       BenchmarkBase (基类)           │    │
    │  └───┬──────┬──────┬────────────────┘    │
    │      │      │      │                      │
    │  ┌───▼────┐ ┌▼──────┐ ┌▼────────────────┐ │
    │  │hnswlib │ │arena- │ │ future library  │ │
    │  │ bench  │ │hnswlib│ │     bench       │ │
    │  └────────┘ └───────┘ └────────────────┘ │
    └──────────────────────────────────────────────┘
         │
         │ BenchmarkResult
         ▼
    ┌─────────────────┐
    │  Result Output  │
    │  (console/json) │
    └─────────────────┘
```

---

## 核心模块详解

### 1. DataLoader 模块

**职责**: 加载不同格式的向量数据集

**设计模式**: 策略模式 (Strategy Pattern) + 工厂模式

```cpp
// 基类定义接口
class FormatAdapter {
  virtual bool CanHandle(const std::string& filepath) = 0;
  virtual std::vector<std::vector<float>> ReadVectors(...) = 0;
};

// 具体实现
class FvecsAdapter : public FormatAdapter { ... };
class IvecsAdapter : public FormatAdapter { ... };
class Hdf5Adapter : public FormatAdapter { ... };

// 主接口
class DataLoader {
  void RegisterAdapter(std::unique_ptr<FormatAdapter> adapter);
  Dataset LoadDataset(const std::string& path);
};
```

**扩展性**: 
- 新增文件格式只需实现新的 FormatAdapter
- 无需修改 DataLoader 核心代码

---

### 2. Benchmark 模块

**职责**: 执行基准测试，收集性能指标

**设计模式**: 模板方法模式 (Template Method Pattern)

```cpp
// 基类定义接口
class BenchmarkBase {
  virtual void Init(...) = 0;
  virtual void BuildIndex(...) = 0;
  virtual std::vector<std::vector<int>> Search(...) = 0;
  virtual std::vector<std::vector<int>> ConcurrentSearch(...) = 0;
};

// 具体实现
class HnswlibBenchmark : public BenchmarkBase { ... };
class ArenaHnswlibBenchmark : public BenchmarkBase { ... };
```

**扩展性**:
- 新增测试库只需实现新的 BenchmarkBase 子类
- 统一的结果收集和输出格式

---

### 3. CLI 模块

**职责**: 解析命令行参数

```cpp
struct CliOptions {
  std::string data_dir;
  std::string config_file;
  std::string output_format;
  bool verbose;
};

class CliParser {
  CliOptions Parse(int argc, char** argv);
};
```

---

### 4. Config 模块

**职责**: 加载和管理配置参数

```cpp
// YAML 配置
benchmark:
  M_values: [16, 32]
  ef_construction: 200
  ef_search_values: [100, 200, 500, 1000]
  k: 10
  num_threads_values: [1, 2, 4, 8]

// C++ 结构
struct BenchmarkParams {
  std::vector<int> M_values;
  int ef_construction;
  std::vector<int> ef_search_values;
  int k;
  std::vector<int> num_threads_values;
};
```

---

## 数据流

```
用户命令行
    │
    ▼
CliParser.Parse()
    │
    ├─► DataLoader.LoadDataset() ──► Dataset
    │         │
    │         ├─► FvecsAdapter (检测 .fvecs)
    │         ├─► IvecsAdapter (检测 .ivecs)
    │         └─► Hdf5Adapter  (检测 .hdf5)
    │
    ├─► ConfigLoader.Load() ──► BenchmarkParams
    │
    ▼
RunBenchmarks(benchmarks, dataset, params)
    │
    ├─► HnswlibBenchmark.Run()
    ├─► ArenaHnswlibBenchmark.Run()
    │
    ▼
PrintResults(results)
```

---

## 扩展点

### 新增文件格式

1. 创建新的适配器类：
```cpp
class MyFormatAdapter : public FormatAdapter {
  bool CanHandle(const std::string& filepath) override {
    return filepath.find(".myformat") != std::string::npos;
  }
  
  std::vector<std::vector<float>> ReadVectors(...) override {
    // 实现读取逻辑
  }
};
```

2. 注册到 DataLoader：
```cpp
loader.RegisterAdapter(std::make_unique<MyFormatAdapter>());
```

---

### 新增测试库

1. 创建新的基准测试类：
```cpp
class MyLibBenchmark : public BenchmarkBase {
  void Init(...) override { /* ... */ }
  void BuildIndex(...) override { /* ... */ }
  std::vector<std::vector<int>> Search(...) override { /* ... */ }
};
```

2. 添加到测试列表：
```cpp
benchmarks.push_back(std::make_unique<MyLibBenchmark>());
```

---

## 文件组织

```
include/
├── common/
│   ├── types.h           # 基础类型
│   └── config.h          # 配置结构
├── loader/
│   ├── data_loader.h     # DataLoader 主接口
│   ├── format_adapter.h  # 适配器基类
│   ├── fvecs_adapter.h   # fvecs 格式
│   ├── ivecs_adapter.h   # ivecs 格式
│   └── hdf5_adapter.h    # HDF5 格式
├── benchmark/
│   ├── benchmark_base.h  # 基准测试基类
│   ├── hnswlib_bench.h   # hnswlib 测试
│   └── arena_hnswlib_bench.h # arena-hnswlib 测试
└── cli/
    └── cli_parser.h      # 命令行解析

src/
├── common/
├── loader/
├── benchmark/
├── cli/
└── main.cpp              # 简洁的入口
```

---

## 依赖管理

| 库 | 用途 | 集成方式 |
|---|---|---|
| yaml-cpp | 配置解析 | FetchContent |
| HighFive | HDF5 读取 | FetchContent (Header-only) |
| googletest | 单元测试 | FetchContent |

---

---

## 测试扩展指南

### 新增测试库

1. **创建测试类** (如 `include/benchmark/mylib_bench.h`):
```cpp
class MyLibBenchmark : public BenchmarkBase {
public:
  void Init(const Dataset& dataset, 
           const BenchmarkParams& params) override;
  void BuildIndex(int M, int ef_construction) override;
  std::vector<std::vector<int>> Search(...) override;
  std::vector<std::vector<int>> ConcurrentSearch(...) override;
};
```

2. **注册到测试列表** (main.cpp):
```cpp
benchmarks.push_back(std::make_unique<MyLibBenchmark>());
```

### 新增测试场景（当前 Google Benchmark 方式）

1. **添加新测试函数**:
```cpp
static void BM_Hnswlib_NewScenario(benchmark::State& state) {
  int param = state.range(0);
  // ... 测试逻辑
  state.counters["metric"] = value;
}
BENCHMARK(BM_Hnswlib_NewScenario)
    ->Args({100})
    ->Args({200});
```

2. **添加新参数组合**:
```cpp
BENCHMARK(BM_Hnswlib_Search)
    ->Args({100})
    ->Args({200})
    ->Args({2000});  // 新增
```

### 统一指标字段（推荐约定）

为确保不同库测试结果可比较，建议统一以下指标：

| 指标 | 键名 | 单位 |
|------|------|------|
| 召回率 | `recall` | 百分比 (0-100) |
| 吞吐量 | `qps` | queries/sec |
| 延迟 | `latency_ms` | 毫秒 |
| 索引时间 | `duration_ms` | 毫秒 |
| 索引大小 | `index_size_mb` | MB |
| 线程数 | `threads` | 整数 |

---

## 后续规划

- **Phase 2**: 添加更多文件格式支持（如 .npy, .bin）
- **Phase 3**: 添加更多向量数据库测试（Milvus, Qdrant）
- **Phase 4**: 添加结果可视化和报告生成
- **Phase 5**: 支持分布式测试和云端部署
