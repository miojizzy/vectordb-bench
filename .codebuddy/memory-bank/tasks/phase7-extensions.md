# Phase 7: 扩展功能

**Status**: Not Started
**Priority**: P1
**Estimated Effort**: 5 days

## Overview

实现过滤搜索测试、流式测试、自定义数据集支持和构建打包。

## Task 7.1: 过滤搜索测试用例

**Status**: Not Started
**File**: `include/vectordb_bench/core/filter_search_case.h`

### Checklist
- [ ] 定义 `FilterSearchCase` 类
- [ ] 实现 int 类型过滤
- [ ] 实现 label 标签过滤
- [ ] 实现过滤条件生成
- [ ] 实现过滤性能对比
- [ ] 实现源文件 `src/core/filter_search_case.cpp`

### Filter Types

| Type | Example | Description |
|------|---------|-------------|
| Int Range | `id >= 1000 AND id < 2000` | 数值范围过滤 |
| Label | `color == 'red'` | 标签过滤 |
| Compound | `price < 100 AND category == 'A'` | 组合过滤 |

### Test Flow

```
1. 生成带标签的测试数据
2. 执行无过滤搜索 (基准)
3. 执行带过滤搜索
4. 记录性能差异
```

### Acceptance Criteria
- 正确执行过滤搜索
- 记录过滤对性能的影响

---

## Task 7.2: 流式测试用例

**Status**: Not Started
**File**: `include/vectordb_bench/core/streaming_case.h`

### Checklist
- [ ] 定义 `StreamingCase` 类
- [ ] 实现持续插入线程
- [ ] 实现并发搜索线程
- [ ] 实现插入速率控制
- [ ] 实现性能监控
- [ ] 实现源文件 `src/core/streaming_case.cpp`

### Test Flow

```
1. 启动插入线程 (固定速率)
2. 启动搜索线程 (多个并发)
3. 持续运行指定时间
4. 记录搜索性能变化
5. 停止所有线程
```

### Configuration

```yaml
streaming:
  insert_rate: 1000  # vectors/sec
  duration: 300      # seconds
  search_concurrency: 10
```

### Acceptance Criteria
- 正确模拟流式场景
- 记录性能随时间变化

---

## Task 7.3: 自定义数据集支持

**Status**: Not Started

### Checklist
- [ ] 实现 Parquet 格式校验
- [ ] 实现 Ground Truth 加载
- [ ] 实现数据集元数据解析
- [ ] 支持压缩 Parquet 文件

### Dataset Format

```
dataset/
├── train.parquet          # 或 train-00-of-10.parquet
├── test.parquet
├── neighbors.parquet      # Ground Truth
└── metadata.json          # 元数据
```

### Metadata Example

```json
{
  "name": "custom_dataset",
  "size": 1000000,
  "dimension": 768,
  "metric_type": "L2",
  "description": "Custom dataset for testing"
}
```

### Acceptance Criteria
- 正确加载自定义数据集
- 验证数据格式正确

---

## Task 7.4: CMake 构建完善

**Status**: Not Started

### Checklist
- [ ] 完善 CMakeLists.txt
- [ ] 添加编译选项配置
- [ ] 添加第三方依赖管理
- [ ] 实现条件编译 (可选数据库)
- [ ] 添加安装目标
- [ ] 创建 CMake config 文件

### Build Options

```cmake
option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOCUMENTATION "Build documentation" OFF)
option(ENABLE_MILVUS "Enable Milvus support" OFF)
option(ENABLE_QDRANT "Enable Qdrant support" OFF)
option(ENABLE_PGVECTOR "Enable PgVector support" OFF)
option(ENABLE_ELASTICSEARCH "Enable Elasticsearch support" OFF)
```

### Acceptance Criteria
- CMake 配置正确
- 支持条件编译

---

## Task 7.5: Docker 支持

**Status**: Not Started

### Checklist
- [ ] 创建 Dockerfile
- [ ] 创建 docker-compose.yml
- [ ] 配置多阶段构建
- [ ] 优化镜像大小

### Dockerfile

```dockerfile
# Build stage
FROM gcc:11 AS builder
WORKDIR /app
COPY . .
RUN cmake -B build && cmake --build build

# Runtime stage
FROM ubuntu:22.04
COPY --from=builder /app/build/vectordb_bench /usr/local/bin/
ENTRYPOINT ["vectordb_bench"]
```

### Acceptance Criteria
- Docker 镜像构建成功
- 容器可正常运行测试

---

## Task 7.6: 安装脚本

**Status**: Not Started

### Checklist
- [ ] 创建 install.sh
- [ ] 检测系统依赖
- [ ] 自动安装第三方库
- [ ] 配置环境变量

### Acceptance Criteria
- 脚本在主流 Linux 发行版运行

---

## Task 7.7: 文档完善

**Status**: Not Started

### Checklist
- [ ] 完善 README.md
- [ ] 创建用户指南
- [ ] 创建开发指南
- [ ] 创建 API 文档

### Documentation Structure

```
docs/
├── user_guide.md      # 用户指南
├── dev_guide.md       # 开发指南
├── api/               # API 文档
├── configuration.md   # 配置说明
└── examples/          # 使用示例
```

### Acceptance Criteria
- 文档完整清晰

---

## Task 7.8: CI/CD 配置

**Status**: Not Started

### Checklist
- [ ] 创建 GitHub Actions workflow
- [ ] 配置自动构建
- [ ] 配置自动测试
- [ ] 配置代码覆盖率
- [ ] 配置 Release 发布

### CI Flow

```yaml
name: CI
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: cmake -B build && cmake --build build
      - name: Test
        run: ctest --test-dir build
```

### Acceptance Criteria
- CI 自动运行
- 测试覆盖率报告生成

---

## Dependencies

- Phase 1-6 完成

## Notes

- 扩展功能可根据实际需求调整优先级
- Docker 支持优先于其他扩展功能
