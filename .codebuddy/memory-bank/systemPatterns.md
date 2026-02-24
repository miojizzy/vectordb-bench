# System Patterns

## Architecture Overview

```
+------------------------+
|     CLI Interface      |  <- 命令行接口
+------------------------+
            |
+------------------------+
|    Benchmark Core      |  <- 核心测试引擎
+------------------------+
            |
    +-------+-------+
    |               |
+-------+     +------------+     +-----------------+
|Dataset|     |   Result   |     | CPU Affinity    |
|Manager|     | Comparator |     | Manager         |
+-------+     +------------+     +-----------------+
    |                |                    |
    |                v                    v
    |    +------------------------+  +----------------+
    |    | Comparison Report      |  | Thread Control |
    |    | Generator              |  | & CPU Binding  |
    |    +------------------------+  +----------------+
    |
+------------------------+
|     DB Clients         |  <- 数据库客户端抽象层
+------------------------+
    |
+---+---+---+---+---+---+
|   |   |   |   |   |   |
| M | Q | E | P |...|   |  <- 具体数据库实现
| i | d | l | g |   |   |
| l | r | a | v |   |   |
| v | a | s | e |   |   |
| u | n | t | c |   |   |
| s | t | i | t |   |   |
|   |   | c | o |   |   |
|   |   |   | r |   |   |
+---+---+---+---+---+---+
```

## Multi-Database Comparison Architecture

```
+---------------------------------------------------+
|               Comparison Runner                    |
+---------------------------------------------------+
           |                    |
           v                    v
+------------------+    +------------------+
|   Test Config    |    |   Shared Dataset |
|   (CPU: 1 core)  |    |   (Loaded Once)  |
+------------------+    +------------------+
           |
    +------+------+
    |      |      |
    v      v      v
+------+ +------+ +------+
|Milvus| |Qdrant| |PgVec|  <- 并行测试多个数据库
|Client| |Client| |tor  |
+------+ +------+ +------+
    |      |      |
    v      v      v
+------------------------+
|   Result Aggregator    |
+------------------------+
            |
            v
+------------------------+
|   Comparison Report    |
|   (QPS/Recall/Latency) |
+------------------------+
```

## Design Patterns

### 1. Abstract Factory Pattern - Database Client Creation

```cpp
class VectorDBFactory {
 public:
  static std::unique_ptr<VectorDB> Create(const std::string& db_type);
};

// Usage
auto client = VectorDBFactory::Create("milvus");
```

### 2. Strategy Pattern - Benchmark Case Execution

```cpp
class BenchmarkCase {
 public:
  virtual CaseResult Run(const BenchmarkConfig& config) = 0;
};

class SearchPerformanceCase : public BenchmarkCase { ... };
class CapacityCase : public BenchmarkCase { ... };
```

### 3. Observer Pattern - Metrics Collection

```cpp
class MetricsObserver {
 public:
  virtual void OnQueryComplete(const QueryResult& result) = 0;
};

class MetricsCollector : public MetricsObserver {
  void OnQueryComplete(const QueryResult& result) override;
};
```

### 4. PIMPL Pattern - Implementation Hiding

```cpp
class MilvusClient : public VectorDB {
 public:
  MilvusClient();
  ~MilvusClient() override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
```

### 5. Template Method Pattern - Test Execution Flow

```cpp
class BenchmarkRunner {
 public:
  void Run() {
    Prepare();      // 准备环境
    LoadDataset();  // 加载数据
    BuildIndex();   // 构建索引
    ExecuteTest();  // 执行测试
    Cleanup();      // 清理资源
  }

 protected:
  virtual void Prepare() = 0;
  virtual void ExecuteTest() = 0;
};
```

## Module Interfaces

### VectorDB Interface

```cpp
class VectorDB {
 public:
  virtual ~VectorDB() = default;

  // Connection
  virtual void Connect(const DBConfig& config) = 0;
  virtual void Disconnect() = 0;
  virtual bool IsConnected() const = 0;

  // Collection Management
  virtual void CreateCollection(const CollectionConfig& config) = 0;
  virtual void DropCollection(const std::string& name) = 0;

  // Data Operations
  virtual void Insert(const std::string& collection,
                      const std::vector<Vector>& vectors,
                      const std::vector<VectorId>& ids) = 0;

  // Index Management
  virtual void CreateIndex(const std::string& collection,
                          const IndexConfig& config) = 0;

  // Search
  virtual std::vector<std::vector<SearchResult>> Search(
      const std::string& collection,
      const std::vector<Vector>& queries,
      int64_t k,
      const SearchParams& params) = 0;
};
```

### Dataset Interface

```cpp
class Dataset {
 public:
  virtual ~Dataset() = default;

  virtual std::string Name() const = 0;
  virtual int64_t Size() const = 0;
  virtual int64_t Dimension() const = 0;
  virtual MetricType GetMetricType() const = 0;

  virtual const Vector& GetTrainVector(int64_t index) const = 0;
  virtual const Vector& GetTestVector(int64_t index) const = 0;
  virtual const std::vector<VectorId>& GetGroundTruth(int64_t query_index) const = 0;
};
```

### BenchmarkCase Interface

```cpp
class BenchmarkCase {
 public:
  virtual ~BenchmarkCase() = default;

  virtual CaseType GetType() const = 0;
  virtual std::string GetName() const = 0;

  virtual void Prepare(VectorDB* db, const Dataset* dataset) = 0;
  virtual CaseResult Run(const BenchmarkConfig& config) = 0;
  virtual void Cleanup() = 0;
};
```

## Data Flow

### Single Database Test Flow

```
Config -> Dataset Loader -> Benchmark Runner -> Metrics Collector -> Report Generator
              |                   |
              v                   v
           Dataset            VectorDB Client
```

### Comparison Test Flow

```
Comparison Config
       |
       v
+----------------+
| Shared Dataset | (Load Once)
+----------------+
       |
       v
+----------------+
| CPU Config 1   | -> Test All DBs -> Aggregate Results
+----------------+
       |
       v
+----------------+
| CPU Config 2   | -> Test All DBs -> Aggregate Results
+----------------+
       |
       v
Comparison Report Generator
```

## Error Handling Strategy

1. **Connection Errors**: 重试机制，指数退避
2. **Query Timeout**: 记录超时，继续后续测试
3. **Memory Exhaustion**: 提前检测，优雅退出
4. **Data Corruption**: 校验检查，报错终止

## Concurrency Model

- **Thread Pool**: 用于并发查询测试
- **CPU Binding**: 线程绑定到指定核心
- **Lock-free Queue**: 用于任务分发（可选）
