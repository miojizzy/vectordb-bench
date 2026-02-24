# Phase 4: 数据库支持

**Status**: Not Started
**Priority**: P0
**Estimated Effort**: 7 days

## Overview

实现数据库客户端工厂和具体的数据库客户端，支持 Milvus、Qdrant、PgVector。

## Task 4.1: 数据库客户端工厂

**Status**: Not Started
**File**: `include/vectordb_bench/clients/client_factory.h`

### Checklist
- [ ] 定义 `VectorDBFactory` 工厂类
- [ ] 实现客户端注册机制
- [ ] 实现 Create() 方法
- [ ] 实现源文件 `src/clients/client_factory.cpp`

### Implementation Details

```cpp
class VectorDBFactory {
 public:
  using CreatorFunc = std::function<std::unique_ptr<VectorDB>()>;
  
  static void Register(const std::string& db_type, CreatorFunc creator);
  static std::unique_ptr<VectorDB> Create(const std::string& db_type);
  static std::vector<std::string> ListSupportedDatabases();
};
```

### Acceptance Criteria
- 支持运行时注册新数据库类型
- 返回正确的客户端实例

---

## Task 4.2: Milvus 客户端 - 连接管理

**Status**: Not Started
**File**: `include/vectordb_bench/clients/milvus_client.h`

### Checklist
- [ ] 定义 `MilvusClient` 类
- [ ] 实现 Connect()
- [ ] 实现 Disconnect()
- [ ] 实现 IsConnected()
- [ ] 实现连接池管理
- [ ] 实现源文件 `src/clients/milvus_client.cpp`

### Dependencies
- libmilvus (Milvus C++ SDK)

### Acceptance Criteria
- 成功连接 Milvus 服务
- 支持认证

---

## Task 4.3: Milvus 客户端 - 集合操作

**Status**: Not Started

### Checklist
- [ ] 实现 CreateCollection()
- [ ] 实现 DropCollection()
- [ ] 实现 HasCollection()
- [ ] 支持指定维度和度量类型
- [ ] 支持分片配置

### Acceptance Criteria
- 正确创建和删除集合

---

## Task 4.4: Milvus 客户端 - 数据操作

**Status**: Not Started

### Checklist
- [ ] 实现 Insert()
- [ ] 实现 InsertBatch()
- [ ] 支持批量插入优化
- [ ] 实现进度回调

### Acceptance Criteria
- 支持大批量数据插入
- 插入性能满足要求

---

## Task 4.5: Milvus 客户端 - 索引管理

**Status**: Not Started

### Checklist
- [ ] 实现 CreateIndex()
- [ ] 实现 DropIndex()
- [ ] 支持 HNSW 索引
- [ ] 支持 IVF_FLAT 索引
- [ ] 支持 IVF_PQ 索引
- [ ] 支持 AutoIndex

### Index Parameters

| Index Type | Parameters |
|------------|------------|
| HNSW | M, efConstruction |
| IVF_FLAT | nlist |
| IVF_PQ | nlist, m, nbits |

### Acceptance Criteria
- 正确创建和删除索引

---

## Task 4.6: Milvus 客户端 - 搜索

**Status**: Not Started

### Checklist
- [ ] 实现 Search()
- [ ] 实现 SearchWithFilter()
- [ ] 支持批量搜索
- [ ] 支持搜索参数配置

### Acceptance Criteria
- 正确返回搜索结果
- 支持过滤条件

---

## Task 4.7: Qdrant 客户端

**Status**: Not Started
**File**: `include/vectordb_bench/clients/qdrant_client.h`

### Checklist
- [ ] 定义 `QdrantClient` 类
- [ ] 实现 gRPC 连接
- [ ] 实现集合操作 (Create/Delete/Exists)
- [ ] 实现数据插入
- [ ] 实现索引配置
- [ ] 实现向量搜索
- [ ] 实现源文件 `src/clients/qdrant_client.cpp`

### Dependencies
- gRPC
- Protobuf

### Acceptance Criteria
- 成功连接 Qdrant 服务
- 所有基本操作正常

---

## Task 4.8: PgVector 客户端

**Status**: Not Started
**File**: `include/vectordb_bench/clients/pgvector_client.h`

### Checklist
- [ ] 定义 `PgVectorClient` 类
- [ ] 实现 libpq 连接
- [ ] 实现表操作 (CREATE TABLE with vector column)
- [ ] 实现数据插入 (INSERT)
- [ ] 实现索引创建 (HNSW, IVFFlat)
- [ ] 实现向量搜索 (SELECT with ORDER BY)
- [ ] 实现源文件 `src/clients/pgvector_client.cpp`

### SQL Examples

```sql
-- Create table
CREATE TABLE vectors (id BIGINT, embedding vector(128));

-- Create HNSW index
CREATE INDEX ON vectors USING hnsw (embedding vector_l2_ops) WITH (m = 16, ef_construction = 64);

-- Search
SELECT id, embedding <=> '[1,2,3,...]' as distance FROM vectors ORDER BY embedding <=> '[1,2,3,...]' LIMIT 100;
```

### Dependencies
- libpq

### Acceptance Criteria
- 成功连接 PostgreSQL
- 正确执行向量操作

---

## Task 4.9: Elasticsearch 客户端 (Optional)

**Status**: Not Started
**File**: `include/vectordb_bench/clients/es_client.h`

### Checklist
- [ ] 定义 `EsClient` 类
- [ ] 实现 HTTP 连接
- [ ] 实现索引操作
- [ ] 实现数据批量插入
- [ ] 实现 kNN 搜索
- [ ] 实现源文件 `src/clients/es_client.cpp`

### Dependencies
- cpr (HTTP client)

---

## Task 4.10: 集成测试

**Status**: Not Started

### Checklist
- [ ] 编写 Milvus 客户端集成测试
- [ ] 编写 Qdrant 客户端集成测试
- [ ] 编写 PgVector 客户端集成测试
- [ ] 使用 Docker Compose 启动测试环境

### Test Environment

```yaml
# docker-compose.yml
services:
  milvus:
    image: milvusdb/milvus:latest
    ports:
      - "19530:19530"
      
  qdrant:
    image: qdrant/qdrant:latest
    ports:
      - "6333:6333"
      
  postgres:
    image: pgvector/pgvector:latest
    ports:
      - "5432:5432"
```

### Dependencies
- Phase 1-3 完成
- Docker 环境

## Notes

- 数据库客户端是最复杂的模块
- 需要实际数据库环境进行集成测试
- 可考虑使用 Docker Compose 搭建测试环境
