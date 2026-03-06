# 使用指南

## 编译

```bash
cd naive
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

编译后会生成可执行文件：
- `hnswlib_bench` - 统一基准测试程序
- `unit_tests` - 单元测试程序

## 运行单元测试

```bash
./unit_tests
```

## 数据准备

### 扩展 Groundtruth

使用 `extend_groundtruth.py` 脚本可以将现有的 groundtruth 从较小的 top-k 扩展到更大的 top-k。

#### 基本用法

```bash
cd naive
python3 scripts/extend_groundtruth.py <data_dir> <new_k> [options]
```

#### 参数说明

- `data_dir`: 数据集目录路径（包含 base.fvecs, query.fvecs, groundtruth.ivecs）
- `new_k`: 新的 top-k 值

#### 可选参数

- `--distance <metric>`: 距离度量方式
  - `euclidean` 或 `l2` - L2 距离（默认）
  - `angular` 或 `cosine` - 角距离
  - `ip` 或 `inner_product` - 内积
- `--workers <n>`: 工作进程数（默认: CPU核心数-1）
- `--force`: 强制重新计算，即使目标文件已存在

#### 示例

```bash
# 将 SIFT 数据集的 groundtruth 从 top-100 扩展到 top-1000
python3 scripts/extend_groundtruth.py data/sift-128-euclidean-topk100 1000

# 将 GloVe 数据集的 groundtruth 扩展到 top-500（角距离）
python3 scripts/extend_groundtruth.py data/glove-50-angular-topk100 500 --distance angular

# 使用 4 个工作进程
python3 scripts/extend_groundtruth.py data/sift-128-euclidean-topk100 1000 --workers 4

# 强制重新计算
python3 scripts/extend_groundtruth.py data/sift-128-euclidean-topk100 1000 --force
```

#### 输出文件

脚本会生成以下文件：
- `groundtruth_<new_k>.ivecs`: 新的 groundtruth 文件
- `groundtruth.ivecs`: 更新后的 groundtruth（top-k=<new_k>）
- `groundtruth.ivecs.bak`: 原始 groundtruth 的备份

#### 性能参考

对于 SIFT 数据集（1M 向量，128 维，10K 查询）：
- 扩展 top-100 → top-1000: 约 6-7 分钟
- 平均每查询: ~40ms
- 吞吐量: ~28M ops/s
- 内存占用: ~500 MB（base 数据）

#### 架构说明

脚本使用多进程并行计算：
- 主进程加载 base/query 数据到内存
- 使用 Linux fork 创建工作进程池（copy-on-write 共享数据）
- 每个工作进程计算部分查询的 top-k
- 使用 NumPy BLAS 加速 + argpartition 算法

## 基本用法

### 1. 查看帮助

```bash
./hnswlib_bench --help
```

### 2. 运行所有测试

```bash
./hnswlib_bench --data_dir ./data/fashion-mnist-784-euclidean
```

这将运行：
- 索引构建测试
- 单线程搜索测试
- 并发搜索测试

### 3. 只运行搜索测试（自动预构建索引）

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib,arena_doublem \
  --benchmark_filter="BM_Search"
```

程序会自动检测没有索引构建测试，并预构建所有需要的索引。

### 4. 并行构建索引

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib,arena_doublem,arena_heuristiconly \
  --build_parallel \
  --benchmark_filter="BM_Search"
```

### 5. 只运行索引构建测试

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --benchmark_filter="BM_IndexBuild"
```

### 6. 运行并发测试

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --benchmark_filter="BM_Search_Concurrent"
```

## 参数说明

### 必需参数

- `--data_dir <path>`: 数据集目录路径

### 可选参数

#### 索引相关
- `--index_types <types>`: 索引类型（逗号分隔）
  - `hnswlib` - 标准 hnswlib
  - `arena_doublem` - arena-hnswlib DoubleM 模式
  - `arena_heuristiconly` - arena-hnswlib HeuristicOnly 模式
  - `arena_heuristicplusclosest` - arena-hnswlib HeuristicPlusClosest 模式
  - 默认: `hnswlib`

- `--M <values>`: HNSW M 参数（逗号分隔）
  - 默认: `16`

- `--ef_construction <values>`: 构建时 ef 参数（逗号分隔）
  - 默认: `200`

#### 测试相关
- `--k <value>`: top-k 值
  - 默认: `100`

- `--ef_search <values>`: 搜索时 ef 参数（逗号分隔）
  - 默认: `10,20,50,100`

- `--num_threads <values>`: 线程数（逗号分隔）
  - 默认: `1,2,4,8`

- `--build_parallel`: 并行构建索引
  - 默认: 否

#### 输出相关
- `--output_file <path>`: 输出文件路径

- `--benchmark_filter <pattern>`: Google Benchmark 过滤器

- `--benchmark_format <format>`: 输出格式
  - `console` - 控制台输出
  - `json` - JSON 格式
  - `csv` - CSV 格式

- `--benchmark_out <file>`: Benchmark 输出文件

## 测试场景示例

### 场景 1: 对比不同索引的搜索性能

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib,arena_doublem,arena_heuristiconly,arena_heuristicplusclosest \
  --benchmark_filter="BM_Search" \
  --build_parallel
```

### 场景 2: 测试不同 M 参数的影响

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib \
  --M 8,16,32,64 \
  --ef_construction 200 \
  --benchmark_filter="BM_Search"
```

### 场景 3: 测试并发性能

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib,arena_doublem \
  --num_threads 1,2,4,8,16 \
  --benchmark_filter="BM_Search_Concurrent" \
  --build_parallel
```

### 场景 4: 输出 JSON 格式结果

```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --benchmark_format=json \
  --benchmark_out=results.json
```

### 场景 5: 使用不同数据集

```bash
# Last.fm (Inner Product)
./hnswlib_bench \
  --data_dir ./data/lastfm-65-angular

# SIFT (L2)
./hnswlib_bench \
  --data_dir ./data/sift-128-euclidean

# GloVe (Inner Product)
./hnswlib_bench \
  --data_dir ./data/glove-128-angular
```

## 性能指标说明

### 索引构建测试 (BM_IndexBuild)
- `duration_ms`: 构建时间（毫秒）
- `index_size_mb`: 索引大小（MB）
- `items_per_second`: 每秒处理的向量数

### 搜索测试 (BM_Search)
- `recall`: 召回率（百分比）
- `ef_search`: 搜索时的 ef 参数
- `items_per_second`: QPS（每秒查询数）

### 并发测试 (BM_Search_Concurrent)
- `recall`: 召回率
- `threads`: 线程数
- `items_per_second`: 并发 QPS

## 故障排除

### 1. 内存不足
减少测试的索引类型或 M 参数值：
```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib \
  --M 16
```

### 2. 构建时间过长
使用并行构建：
```bash
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --build_parallel \
  --benchmark_filter="BM_Search"
```

### 3. 只想测试特定参数
使用 Google Benchmark 过滤器：
```bash
# 只测试 M=16 的搜索
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --benchmark_filter="BM_Search.*16/200/"
```

## 高级用法

### 1. 自定义测试脚本

```bash
#!/bin/bash

DATA_DIR="./data/fashion-mnist-784-euclidean"
RESULTS_DIR="./results/$(date +%Y%m%d_%H%M%S)"

mkdir -p $RESULTS_DIR

# Test different index types
for INDEX_TYPE in hnswlib arena_doublem arena_heuristiconly; do
  echo "Testing $INDEX_TYPE..."
  ./hnswlib_bench \
    --data_dir $DATA_DIR \
    --index_types $INDEX_TYPE \
    --benchmark_filter="BM_Search" \
    --build_parallel \
    --benchmark_format=json \
    --benchmark_out=$RESULTS_DIR/${INDEX_TYPE}.json
done

echo "Results saved to $RESULTS_DIR"
```

### 2. 批量测试多个数据集

```bash
#!/bin/bash

DATASETS=(
  "fashion-mnist-784-euclidean"
  "lastfm-65-angular"
  "sift-128-euclidean"
)

for DATASET in "${DATASETS[@]}"; do
  echo "Testing $DATASET..."
  ./hnswlib_bench \
    --data_dir ./data/$DATASET \
    --index_types hnswlib,arena_doublem \
    --benchmark_filter="BM_Search" \
    --build_parallel \
    --benchmark_format=json \
    --benchmark_out=./results/${DATASET}.json
done
```
