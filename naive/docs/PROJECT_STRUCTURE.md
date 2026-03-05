# 项目结构

## 目录结构

```
naive/
├── CMakeLists.txt              # 主构建配置
├── README.md                   # 项目概述
├── build/                      # 构建输出目录
├── build_tests/                # 单元测试构建目录
├── data/                       # 数据集存储
├── results/                    # 基准测试结果
├── docs/                       # 文档
│   └── USAGE_GUIDE.md          # 使用指南
├── include/                    # 公共头文件
│   ├── arena_hnswlib_wrapper.h
│   ├── benchmark_config.h
│   ├── cli_parser.h
│   ├── dataset_loader.h
│   ├── hnswlib_wrapper.h
│   ├── index_manager.h
│   └── index_wrapper.h
├── src/                        # 源文件
│   ├── main.cpp                # 主入口
│   ├── cli_parser.cpp          # 命令行解析器
│   ├── dataset_loader.cpp      # 数据集加载器
│   ├── index_wrapper.cpp       # 索引包装器工具
│   ├── hnswlib_wrapper.cpp     # hnswlib 实现
│   ├── arena_hnswlib_wrapper.cpp # arena-hnswlib 实现
│   ├── index_manager.cpp       # 索引缓存管理器
│   ├── benchmark_index_build.cpp # 索引构建基准测试
│   ├── benchmark_search.cpp    # 搜索基准测试
│   └── benchmark_concurrent.cpp # 并发基准测试
├── tests/                      # 单元测试
│   ├── CMakeLists.txt          # 测试构建配置
│   ├── README.md               # 测试文档
│   ├── cli_parser_test.cpp
│   ├── index_wrapper_test.cpp
│   ├── hnswlib_wrapper_test.cpp
│   ├── arena_hnswlib_wrapper_test.cpp
│   └── index_manager_test.cpp
├── scripts/                    # 工具脚本
│   ├── download_datasets.sh    # 下载数据集
│   ├── generate_dataset.py     # 生成测试数据集
│   ├── generate_dataset.sh     # 生成测试数据集脚本
│   └── convert_hdf5_to_fvecs.py # HDF5 格式转换
└── third_party/                # 第三方库
    ├── hnswlib/
    └── arena-hnswlib/
```

## 构建命令

### 构建主程序

```bash
cd naive
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 构建并运行单元测试

```bash
cd naive
mkdir -p build_tests && cd build_tests
cmake ../tests
make -j$(nproc)
./unit_tests
```

## 运行基准测试

```bash
# 运行所有基准测试
./hnswlib_bench --data_dir ./data/fashion-mnist-784-euclidean

# 只运行搜索测试
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --benchmark_filter="BM_Search"

# 指定索引类型
./hnswlib_bench \
  --data_dir ./data/fashion-mnist-784-euclidean \
  --index_types hnswlib,arena_doublem
```

## 索引类型

| 名称 | 说明 |
|-----|------|
| `hnswlib` | 标准 hnswlib |
| `arena_doublem` | arena-hnswlib DoubleM 模式 |
| `arena_heuristiconly` | arena-hnswlib HeuristicOnly 模式 |
| `arena_heuristicplusclosest` | arena-hnswlib HeuristicPlusClosest 模式 |

## 基准测试名称

| 测试类型 | 名称格式 | 示例 |
|---------|---------|------|
| 索引构建 | `BM_IndexBuild/<type>` | `BM_IndexBuild/HNSW` |
| 单线程搜索 | `BM_Search/<type>` | `BM_Search/DoubleM` |
| 并发搜索 | `BM_Search_Concurrent/<type>` | `BM_Search_Concurrent/HeurOnly` |

## 数据集命名规范

目录格式：`<name>-<dimension>-<metric>-topk<k>`

示例：
- `sift-128-euclidean-topk100`
- `glove-50-angular-topk100`
- `test-64-euclidean-topk100`

必需文件：
- `base.fvecs` - 训练向量
- `query.fvecs` - 查询向量
- `groundtruth.ivecs` - 真实标签
