# Phase 3: 测试用例

**Status**: Not Started
**Priority**: P0
**Estimated Effort**: 5 days

## Overview

实现数据集加载器和各种测试用例，包括搜索性能测试和容量测试。

## Task 3.1: 数据集加载器基类

**Status**: Not Started
**File**: `include/vectordb_bench/dataset/dataset_loader.h`

### Checklist
- [ ] 定义 `DatasetLoader` 工厂类
- [ ] 定义数据集注册机制
- [ ] 实现数据集类型检测
- [ ] 实现源文件 `src/dataset/dataset_loader.cpp`

### Acceptance Criteria
- 支持按名称或路径加载数据集

---

## Task 3.2: SIFT 数据集

**Status**: Not Started
**File**: `include/vectordb_bench/dataset/sift_dataset.h`

### Checklist
- [ ] 定义 `SiftDataset` 类
- [ ] 实现数据文件加载 (train.fvecs, test.fvecs, groundtruth.ivecs)
- [ ] 实现 fvecs/ivecs 格式解析
- [ ] 实现内存映射加载（大数据集）
- [ ] 实现源文件 `src/dataset/sift_dataset.cpp`

### Data Format

```
SIFT 数据集文件格式:
- train.fvecs: 训练向量
- test.fvecs: 测试向量  
- groundtruth.ivecs: 最近邻结果

fvecs/ivecs 格式:
[维度(int)] [向量数据(float/int)*维度] ...
```

### Acceptance Criteria
- 正确解析 SIFT 数据集格式
- 支持 SIFT 1M 和 SIFT 10M

---

## Task 3.3: GIST 数据集

**Status**: Not Started
**File**: `include/vectordb_bench/dataset/gist_dataset.h`

### Checklist
- [ ] 定义 `GistDataset` 类
- [ ] 实现数据文件加载
- [ ] 实现源文件 `src/dataset/gist_dataset.cpp`

### Acceptance Criteria
- 正确解析 GIST 数据集格式

---

## Task 3.4: 自定义 Parquet 数据集

**Status**: Not Started
**File**: `include/vectordb_bench/dataset/custom_dataset.h`

### Checklist
- [ ] 定义 `CustomDataset` 类
- [ ] 实现 Parquet 文件读取
- [ ] 实现数据格式校验
  - [ ] train.parquet: id (int), emb (float array)
  - [ ] test.parquet: id (int), emb (float array)
  - [ ] neighbors.parquet: id (int), neighbors_id (int array)
- [ ] 支持分片文件命名 (train-01-of-10.parquet)
- [ ] 实现源文件 `src/dataset/custom_dataset.cpp`

### Dependencies
- Arrow/Parquet 库

### Acceptance Criteria
- 支持自定义 Parquet 格式数据集

---

## Task 3.5: 指标收集器

**Status**: Not Started
**File**: `include/vectordb_bench/core/metrics_collector.h`

### Checklist
- [ ] 定义 `MetricsCollector` 类
  - [ ] RecordLatency(): 记录延迟
  - [ ] RecordBatchLatency(): 批量记录
  - [ ] Calculate(): 计算指标
  - [ ] Reset(): 重置
- [ ] 实现线程安全存储
- [ ] 实现百分位计算 (p50, p90, p99)
- [ ] 实现 QPS 计算
- [ ] 实现源文件 `src/core/metrics_collector.cpp`

### Implementation Details

```cpp
struct PerformanceMetrics {
  double qps;
  double latency_p50;
  double latency_p90;
  double latency_p99;
  double latency_avg;
  int64_t total_queries;
  std::chrono::microseconds total_time;
};
```

### Acceptance Criteria
- 线程安全
- 百分位计算准确

---

## Task 3.6: 搜索性能测试用例

**Status**: Not Started
**File**: `include/vectordb_bench/core/search_performance_case.h`

### Checklist
- [ ] 定义 `SearchPerformanceCase` 类
- [ ] 实现 Prepare(): 准备测试环境
- [ ] 实现 RunSerialSearch(): 串行搜索测试
- [ ] 实现 RunConcurrentSearch(): 并发搜索测试
- [ ] 实现 CalculateRecall(): 召回率计算
- [ ] 实现 Cleanup(): 清理资源
- [ ] 实现源文件 `src/core/search_performance_case.cpp`

### Test Flow

```
1. Prepare
   - 连接数据库
   - 创建集合
   - 加载数据集
   
2. Run
   - 串行搜索: 记录延迟、QPS、召回率
   - 并发搜索: 多种并发级别测试
   
3. Cleanup
   - 删除测试数据
   - 断开连接
```

### Acceptance Criteria
- 正确执行搜索性能测试
- 输出完整性能指标

---

## Task 3.7: 容量测试用例

**Status**: Not Started
**File**: `include/vectordb_bench/core/capacity_case.h`

### Checklist
- [ ] 定义 `CapacityCase` 类
- [ ] 实现 Prepare(): 准备测试环境
- [ ] 实现 InsertUntilFull(): 持续插入直到内存满
- [ ] 实现 Run(): 执行测试
- [ ] 实现 Cleanup(): 清理资源
- [ ] 实现内存监控
- [ ] 实现源文件 `src/core/capacity_case.cpp`

### Test Flow

```
1. 开始插入向量
2. 监控内存使用
3. 记录成功插入数量
4. 检测到内存满时停止
5. 报告最大容量
```

### Acceptance Criteria
- 正确检测内存满情况
- 记录准确的最大容量

---

## Task 3.8: 结果结构定义

**Status**: Not Started
**File**: `include/vectordb_bench/common/result.h`

### Checklist
- [ ] 定义 `CaseResult` 结构体
- [ ] 定义 `BenchmarkResult` 结构体
- [ ] 定义 `DbTestResult` 结构体
- [ ] 实现 JSON 序列化
- [ ] 实现源文件 `src/common/result.cpp`

### Acceptance Criteria
- 结果可序列化为 JSON

---

## Task 3.9: 单元测试

**Status**: Not Started

### Checklist
- [ ] 编写数据集加载测试
- [ ] 编写指标收集器测试
- [ ] 编写召回率计算测试

### Dependencies
- Phase 1 完成
- Phase 2 完成

## Notes

- 数据集加载器需要支持内存映射以处理大数据集
- 召回率计算需要验证正确性
