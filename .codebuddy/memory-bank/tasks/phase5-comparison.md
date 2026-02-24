# Phase 5: 多数据库对比

**Status**: Not Started
**Priority**: P0
**Estimated Effort**: 4 days

## Overview

实现多数据库对比测试功能，支持同一数据集测试多个数据库，并生成对比报告。

## Task 5.1: 对比结果结构

**Status**: Not Started
**File**: `include/vectordb_bench/report/comparison_report.h`

### Checklist
- [ ] 定义 `MetricComparison` 结构体
  - [ ] metric_name: 指标名称
  - [ ] unit: 单位
  - [ ] db_metrics: 各数据库指标值
  - [ ] best_db: 最佳数据库
- [ ] 定义 `OverallScore` 结构体
  - [ ] db_name: 数据库名
  - [ ] total_score: 总分
  - [ ] rank: 排名
  - [ ] metric_scores: 各指标得分
- [ ] 定义 `CpuConfigResult` 结构体
- [ ] 定义 `CrossCpuComparison` 结构体
- [ ] 定义 `ComparisonReport` 完整报告结构
- [ ] 实现源文件 `src/report/comparison_report.cpp`

### Acceptance Criteria
- 结构完整覆盖对比所需信息

---

## Task 5.2: 对比运行器

**Status**: Not Started
**File**: `include/vectordb_bench/core/comparison_runner.h`

### Checklist
- [ ] 定义 `ComparisonRunner` 类
  - [ ] AddDatabase(): 添加数据库配置
  - [ ] SetDataset(): 设置共享数据集
  - [ ] RunComparison(): 执行对比测试
- [ ] 实现数据库配置管理
- [ ] 实现共享数据集加载
- [ ] 实现顺序测试调度
- [ ] 实现结果聚合
- [ ] 实现源文件 `src/core/comparison_runner.cpp`

### Test Flow

```
1. 加载共享数据集 (Load Once)
2. For each CPU Config:
   For each Database:
     a. 配置 CPU 资源
     b. 执行测试用例
     c. 收集结果
     d. 清理环境
3. 聚合结果
4. 生成报告
```

### Acceptance Criteria
- 正确执行多数据库对比
- 避免资源竞争

---

## Task 5.3: 结果聚合器

**Status**: Not Started

### Checklist
- [ ] 实现结果收集
- [ ] 实现结果验证
- [ ] 实现异常处理

### Acceptance Criteria
- 正确聚合多个数据库的结果

---

## Task 5.4: 排名计算

**Status**: Not Started

### Checklist
- [ ] 实现指标归一化
- [ ] 实现单指标排名
- [ ] 实现综合评分算法
- [ ] 实现扩展效率计算

### Scoring Algorithm

```cpp
// 归一化得分
double NormalizeScore(double value, double best_value, 
                      bool higher_better = true) {
  if (higher_better) {
    return (value / best_value) * 100.0;
  } else {
    return (best_value / value) * 100.0;
  }
}

// 扩展效率
double CalculateScalingEfficiency(double single_core_qps, 
                                  double multi_core_qps, 
                                  int num_cores) {
  double ideal_qps = single_core_qps * num_cores;
  return multi_core_qps / ideal_qps * 100.0;
}
```

### Weights Configuration

| Metric | Weight |
|--------|--------|
| QPS | 40% |
| Latency P99 | 30% |
| Recall | 30% |

### Acceptance Criteria
- 排名算法正确
- 扩展效率计算准确

---

## Task 5.5: 综合评分

**Status**: Not Started

### Checklist
- [ ] 定义评分权重配置
- [ ] 实现加权评分计算
- [ ] 实现排名生成

### Acceptance Criteria
- 综合评分合理反映性能

---

## Task 5.6: 单元测试

**Status**: Not Started

### Checklist
- [ ] 编写排名计算测试
- [ ] 编写扩展效率测试
- [ ] 编写综合评分测试
- [ ] 编写对比运行器测试

### Dependencies
- Phase 1-4 完成

## Notes

- 对比测试需要稳定的环境以保证公平性
- 考虑预热时间，避免冷启动影响
