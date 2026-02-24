# Phase 1: 核心框架

**Status**: Not Started
**Priority**: P0
**Estimated Effort**: 5 days

## Overview

搭建项目基础框架，包括类型定义、配置系统、接口抽象、日志和 CLI 工具。

## Task 1.1: 项目目录结构

**Status**: Not Started

### Checklist
- [ ] 创建 `include/vectordb_bench/` 目录结构
- [ ] 创建 `src/` 目录结构
- [ ] 创建 `tests/` 目录结构
- [ ] 创建 `config/examples/` 目录
- [ ] 创建根目录 `CMakeLists.txt`
- [ ] 创建 `src/CMakeLists.txt`
- [ ] 创建 `tests/CMakeLists.txt`

### Acceptance Criteria
- 项目目录结构符合 techContext.md 定义
- CMake 可以成功配置空项目

---

## Task 1.2: 基础类型定义

**Status**: Not Started
**File**: `include/vectordb_bench/common/types.h`

### Checklist
- [ ] 定义 `Vector` 类型 (std::vector<float>)
- [ ] 定义 `VectorId` 类型 (int64_t)
- [ ] 定义 `Distance` 类型 (float)
- [ ] 定义 `MetricType` 枚举 (kL2, kCosine, kInnerProduct, kHamming)
- [ ] 定义 `SearchResult` 结构体
- [ ] 定义 `PerformanceMetrics` 结构体
- [ ] 实现源文件 `src/common/types.cpp`

### Acceptance Criteria
- 所有类型定义符合 Google C++ 规范
- 编译通过无警告

---

## Task 1.3: 异常类定义

**Status**: Not Started
**File**: `include/vectordb_bench/common/exception.h`

### Checklist
- [ ] 定义 `VectorDBBenchException` 基类
- [ ] 定义 `ConfigException` 配置异常
- [ ] 定义 `ConnectionException` 连接异常
- [ ] 定义 `OperationException` 操作异常
- [ ] 定义 `TimeoutException` 超时异常
- [ ] 实现源文件 `src/common/exception.cpp`

### Acceptance Criteria
- 异常类继承自 std::exception
- 提供有用的错误信息

---

## Task 1.4: 配置系统

**Status**: Not Started
**File**: `include/vectordb_bench/common/config.h`

### Checklist
- [ ] 定义 `DBConfig` 结构体
- [ ] 定义 `CollectionConfig` 结构体
- [ ] 定义 `IndexConfig` 结构体
- [ ] 定义 `SearchParams` 结构体
- [ ] 定义 `CpuConfig` 结构体（含预设方法）
- [ ] 定义 `BenchmarkConfig` 结构体
- [ ] 定义 `ComparisonConfig` 结构体
- [ ] 实现 YAML 配置文件解析
- [ ] 实现配置验证逻辑
- [ ] 实现源文件 `src/common/config.cpp`

### Dependencies
- yaml-cpp

### Acceptance Criteria
- 支持 YAML 配置文件加载
- 配置验证返回有意义的错误信息

---

## Task 1.5: 接口定义 - VectorDB

**Status**: Not Started
**File**: `include/vectordb_bench/interface/vector_db.h`

### Checklist
- [ ] 定义 `VectorDB` 抽象类
- [ ] 定义连接管理接口 (Connect, Disconnect, IsConnected)
- [ ] 定义集合管理接口 (CreateCollection, DropCollection, HasCollection)
- [ ] 定义数据操作接口 (Insert, InsertBatch)
- [ ] 定义索引管理接口 (CreateIndex, DropIndex)
- [ ] 定义搜索接口 (Search, SearchWithFilter)
- [ ] 定义统计接口 (GetStats)

### Acceptance Criteria
- 接口覆盖所有必要操作
- 纯虚函数定义清晰

---

## Task 1.6: 接口定义 - Dataset

**Status**: Not Started
**File**: `include/vectordb_bench/interface/dataset.h`

### Checklist
- [ ] 定义 `Dataset` 抽象类
- [ ] 定义元数据接口 (Name, Size, Dimension, GetMetricType)
- [ ] 定义数据访问接口 (GetTrainVector, GetTestVector, GetGroundTruth)
- [ ] 定义批量访问接口 (GetTrainVectors, GetTestVectors)
- [ ] 定义大小接口 (TrainSize, TestSize)

### Acceptance Criteria
- 接口支持流式访问大数据集

---

## Task 1.7: 接口定义 - BenchmarkCase

**Status**: Not Started
**File**: `include/vectordb_bench/interface/benchmark_case.h`

### Checklist
- [ ] 定义 `CaseType` 枚举
- [ ] 定义 `BenchmarkCase` 抽象类
- [ ] 定义元数据接口 (GetType, GetName, GetDescription)
- [ ] 定义生命周期接口 (Prepare, Run, Cleanup)

### Acceptance Criteria
- 接口设计支持测试用例扩展

---

## Task 1.8: 日志系统

**Status**: Not Started

### Checklist
- [ ] 添加 spdlog 依赖
- [ ] 创建日志配置头文件 `include/vectordb_bench/common/logger.h`
- [ ] 实现日志初始化函数
- [ ] 配置日志格式
- [ ] 实现日志级别控制
- [ ] 实现源文件 `src/common/logger.cpp`

### Dependencies
- spdlog

### Acceptance Criteria
- 支持多级别日志 (trace, debug, info, warn, error)
- 支持控制台和文件输出

---

## Task 1.9: CLI 工具

**Status**: Not Started
**File**: `src/main.cpp`

### Checklist
- [ ] 添加 CLI11 依赖
- [ ] 定义 CLI 主类
- [ ] 实现 --help 和 --version
- [ ] 实现子命令框架
- [ ] 实现 milvus 子命令
- [ ] 实现 qdrant 子命令
- [ ] 实现 compare 子命令
- [ ] 实现 batch 子命令
- [ ] 实现配置文件加载 (--config)

### Dependencies
- CLI11

### Acceptance Criteria
- CLI 参数解析正确
- 子命令可以独立执行

---

## Task 1.10: 单元测试框架

**Status**: Not Started

### Checklist
- [ ] 添加 googletest 依赖
- [ ] 创建测试目录结构
- [ ] 创建测试主入口 `tests/main_test.cpp`
- [ ] 编写 types.h 单元测试
- [ ] 编写 config.h 单元测试
- [ ] 编写 exception.h 单元测试

### Dependencies
- googletest

### Acceptance Criteria
- 测试覆盖率 > 80%

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| yaml-cpp | >= 0.7 | Configuration parsing |
| CLI11 | >= 2.3 | Command line parsing |
| spdlog | >= 1.9 | Logging |
| googletest | >= 1.11 | Unit testing |

## Notes

- Phase 1 是项目基础，必须先完成
- 所有代码遵循 Google C++ 编码规范
