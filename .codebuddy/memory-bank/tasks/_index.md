# Tasks Index

## 当前阶段

| Phase | Status | Description |
|-------|--------|-------------|
| Phase 0 | ✅ Completed | Naive Baseline - 基础基准测试 |
| Phase 1 | 🚧 Not Started | 核心框架重构 - 模块化架构 |

---

## Phase 0: Naive Baseline ✅

**状态**: 已完成

**目标**: 建立最简单的基准测试框架

**主要成果**:
- ✅ 项目目录结构
- ✅ hnswlib 和 arena-hnswlib 集成
- ✅ fvecs/ivecs 文件读取
- ✅ Google Benchmark 集成
- ✅ 基础性能测试
- ✅ Recall 计算

**详细文档**: [phase0-naive-baseline.md](./phase0-naive-baseline.md)

---

## Phase 1: 核心框架重构 🚧

**状态**: 未开始

**目标**: 建立模块化、可扩展的架构

**设计原则**:
1. **DataLoader 模块化** - 支持多种文件格式的独立适配器
2. **测试逻辑分离** - 每个三方库独立测试模块
3. **命令行解析独立** - main 函数保持简洁

**主要任务**:

### Task 1.x: 基础模块
- [ ] Task 1.1: 基础类型定义
- [ ] Task 1.2: 配置系统

### Task 1.x: DataLoader 模块
- [ ] Task 1.3: 格式适配器接口
- [ ] Task 1.4: fvecs 适配器
- [ ] Task 1.5: ivecs 适配器
- [ ] Task 1.6: HDF5 适配器
- [ ] Task 1.7: DataLoader 主接口

### Task 1.x: Benchmark 模块
- [ ] Task 1.8: 基准测试基类
- [ ] Task 1.9: hnswlib 测试实现
- [ ] Task 1.10: arena-hnswlib 测试实现

### Task 1.x: CLI 模块
- [ ] Task 1.11: 命令行解析器
- [ ] Task 1.12: 重构 main.cpp

**详细文档**: [phase1-core-framework.md](./phase1-core-framework.md)

---

## 架构设计

详见 [architecture.md](../architecture.md)

### 核心模块

```
DataLoader 模块
├── FormatAdapter (基类)
├── FvecsAdapter
├── IvecsAdapter
└── Hdf5Adapter

Benchmark 模块
├── BenchmarkBase (基类)
├── HnswlibBenchmark
└── ArenaHnswlibBenchmark

CLI 模块
└── CliParser

Config 模块
└── BenchmarkParams
```

---

## 未来规划

### Phase 2: 功能扩展
- 更多文件格式支持 (.npy, .bin)
- 更多向量数据库测试 (Milvus, Qdrant)
- 结果输出格式 (JSON, CSV)

### Phase 3: 性能优化
- SIMD 优化
- 多线程优化
- 内存优化

### Phase 4: 可视化与报告
- 结果可视化
- 自动生成报告
- 性能对比图表

---

## 扩展指南

### 新增测试库

```
1. include/benchmark/mylib_bench.h  → 继承 BenchmarkBase
2. src/benchmark/mylib_bench.cpp    → 实现测试逻辑
3. CMakeLists.txt                   → 添加源文件
4. main.cpp                         → 注册到测试列表
```

### 新增测试场景

```cpp
// 在 main.cpp 中添加
static void BM_MyLib_NewScenario(benchmark::State& state) {
  // ...
}
BENCHMARK(BM_MyLib_NewScenario)->Args({...});
```

### 指标字段约定

| 操作类型 | 必需指标 |
|----------|----------|
| IndexConstruction | duration_ms, index_size_mb |
| Search | recall, qps, latency_ms |
| ConcurrentSearch | recall, qps, threads |

---

## 技术栈

| 类别 | 技术 |
|-----|------|
| 语言 | C++17 |
| 构建 | CMake 3.16+ |
| 测试 | Google Benchmark, Google Test |
| 配置 | yaml-cpp |
| HDF5 | HighFive |
| 日志 | spdlog (可选) |
