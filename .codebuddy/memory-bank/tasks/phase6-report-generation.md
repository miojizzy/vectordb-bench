# Phase 6: 报告生成

**Status**: Not Started
**Priority**: P1
**Estimated Effort**: 4 days

## Overview

实现多种格式的报告输出，包括 JSON、CSV、Markdown 和 HTML，并支持图表生成。

## Task 6.1: JSON 报告生成

**Status**: Not Started
**File**: `include/vectordb_bench/report/report_generator.h`

### Checklist
- [ ] 定义 `ReportFormat` 枚举
- [ ] 定义 `ReportGenerator` 类
- [ ] 实现 JSON 格式输出
- [ ] 实现 JSON 美化格式化
- [ ] 实现源文件 `src/report/report_generator.cpp`

### Dependencies
- nlohmann/json

### Output Example

```json
{
  "test_id": "comparison_20240101",
  "timestamp": "2024-01-01T10:00:00Z",
  "environment": {
    "os": "Linux 5.15.0",
    "cpu": "Intel Xeon Platinum 8375C",
    "cores": 32,
    "memory_mb": 65536
  },
  "results": [...]
}
```

### Acceptance Criteria
- JSON 格式正确可解析

---

## Task 6.2: CSV 报告生成

**Status**: Not Started

### Checklist
- [ ] 实现 CSV 格式输出
- [ ] 支持结果表格化
- [ ] 处理特殊字符转义

### Output Example

```csv
Database,QPS,Latency_P99,Recall,Score
Milvus,28000,12,99.2,95.2
Qdrant,25000,15,99.1,89.5
```

### Acceptance Criteria
- CSV 可导入 Excel

---

## Task 6.3: Markdown 报告生成

**Status**: Not Started

### Checklist
- [ ] 实现 Markdown 格式输出
- [ ] 生成结果表格
- [ ] 生成摘要部分

### Output Example

```markdown
# VectorDB 性能对比报告

## 测试环境
- CPU: Intel Xeon Platinum 8375C (32 cores)
- 内存: 64 GB

## 结果排名

| 排名 | 数据库 | QPS | 延迟P99 | 召回率 |
|-----|--------|-----|---------|--------|
| 1 | Milvus | 28,000 | 12ms | 99.2% |
| 2 | Qdrant | 25,000 | 15ms | 99.1% |
```

### Acceptance Criteria
- Markdown 格式正确
- 可在 GitHub 正确渲染

---

## Task 6.4: HTML 报告生成

**Status**: Not Started

### Checklist
- [ ] 定义 HTML 模板
- [ ] 实现 CSS 样式
- [ ] 实现结果表格渲染
- [ ] 实现响应式布局

### HTML Structure

```html
<!DOCTYPE html>
<html>
<head>
  <title>VectorDB Benchmark Report</title>
  <style>/* styles */</style>
</head>
<body>
  <h1>VectorDB 性能对比报告</h1>
  <section id="environment">...</section>
  <section id="results">...</section>
  <section id="charts">...</section>
</body>
</html>
```

### Acceptance Criteria
- HTML 页面美观
- 支持移动端查看

---

## Task 6.5: 图表生成

**Status**: Not Started
**File**: `include/vectordb_bench/report/chart_builder.h`

### Checklist
- [ ] 定义 `ChartBuilder` 类
- [ ] 实现 QPS 对比柱状图
- [ ] 实现延迟分布图
- [ ] 实现 CPU 扩展性图表
- [ ] 实现召回率对比图
- [ ] 实现雷达图（综合对比）
- [ ] 实现源文件 `src/report/chart_builder.cpp`

### Chart Types

| Chart | Type | Purpose |
|-------|------|---------|
| QPS Comparison | Bar Chart | 数据库性能对比 |
| Latency Distribution | Box Plot | 延迟分布展示 |
| CPU Scaling | Line Chart | 多核扩展性 |
| Radar Chart | Radar | 综合对比 |

### Dependencies
- Plotly.js (前端图表库)

### Implementation Approach

生成包含 Plotly.js 数据的 HTML，在浏览器端渲染图表：

```cpp
std::string ChartBuilder::BuildQpsChart(const ComparisonReport& report) {
  // 生成 Plotly.js 需要的数据 JSON
  nlohmann::json data;
  for (const auto& db : report.db_results) {
    data["x"].push_back(db.db_name);
    data["y"].push_back(db.metrics.qps);
  }
  
  // 生成 HTML 片段
  return fmt::format(R"(
    <div id="qps-chart"></div>
    <script>
      Plotly.newPlot('qps-chart', [{{
        x: {},
        y: {},
        type: 'bar'
      }}]);
    </script>
  )", data["x"].dump(), data["y"].dump());
}
```

### Acceptance Criteria
- 图表渲染正确
- 支持交互（缩放、hover）

---

## Task 6.6: 报告模板系统

**Status**: Not Started

### Checklist
- [ ] 定义模板文件格式
- [ ] 实现模板变量替换
- [ ] 支持自定义模板

### Acceptance Criteria
- 用户可自定义报告格式

---

## Task 6.7: 单元测试

**Status**: Not Started

### Checklist
- [ ] 编写 JSON 生成测试
- [ ] 编写 CSV 生成测试
- [ ] 编写 Markdown 生成测试
- [ ] 编写 HTML 生成测试

### Dependencies
- Phase 5 完成

## Notes

- HTML 报告优先支持，使用 Plotly.js 实现图表
- 后续可考虑支持 PDF 导出
