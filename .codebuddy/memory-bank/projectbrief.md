# Project Brief

## Project Overview

VectorDBBench C++ 版本是一个高性能向量数据库基准测试工具，参考 Python 版本的 VectorDBBench 实现，旨在提供高性能、可扩展的向量数据库性能测试能力。

## Core Objectives

1. **高性能**：利用 C++ 的性能优势，提供更高效的测试能力
2. **可扩展**：支持多种向量数据库，易于添加新的数据库支持
3. **可复现**：提供一致的测试环境和结果
4. **易用性**：提供命令行工具和配置文件支持
5. **CPU 资源控制**：支持指定 CPU 核心数和线程数，测试单核/多核场景
6. **多数据库对比**：同一数据集测试多种数据库，生成对比报告

## Target Users

- 向量数据库开发者：验证性能优化效果
- 算法工程师：评估不同数据库的适用性
- 运维人员：容量规划和性能调优
- 研究人员：学术研究和论文实验

## Project Phases

### Phase 0: Naive Baseline (Priority: P0)

**独立的基础版本**，用于验证技术路线和建立性能基准。

| Item | Description |
|------|-------------|
| 测试目标 | hnswlib (header-only HNSW 库) |
| 测试框架 | Google Benchmark |
| 数据集 | SIFT Small (500K, 128维) |
| 代码位置 | `naive/` 目录 |
| 独立性 | 完全独立，后续迭代不涉及 |

**Phase 0 目标**:
1. 验证 HNSW 性能测试技术路线
2. 建立性能数据基准
3. 提供可快速迭代的最小实现

### Phase 1-7: 完整框架

在 Phase 0 验证成功后，构建完整的向量数据库基准测试框架。

## Key Features

### Benchmark Types

| Type | Description |
|------|-------------|
| Capacity | 容量测试，测试数据库加载能力 |
| SearchPerformance | 搜索性能测试，串行/并发搜索性能 |
| FilterSearch | 过滤搜索测试，带过滤条件的搜索 |
| Streaming | 流式测试，数据持续写入下的搜索性能 |

### Supported Databases

- Milvus
- Qdrant
- PgVector
- Elasticsearch
- Weaviate
- Pinecone
- Redis
- Chroma

### CPU Control Features

- 单核/多核性能测试
- 指定核心绑定
- 线程数控制
- 扩展效率分析

### Comparison Features

- 多数据库并行对比
- 多 CPU 配置对比
- 自动排名和评分
- 可视化报告生成

## Project Constraints

- 使用 C++17 标准
- 遵循 Google C++ 编码规范
- 支持 Linux 平台（Windows 可选）
- 最小化外部依赖
