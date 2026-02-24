# Phase 2: CPU 资源控制

**Status**: Not Started
**Priority**: P0
**Estimated Effort**: 3 days

## Overview

实现 CPU 亲和性控制，支持指定核心数和线程绑定，为性能测试提供资源控制能力。

## Task 2.1: CPU 信息采集

**Status**: Not Started
**File**: `include/vectordb_bench/core/cpu_affinity_manager.h`

### Checklist
- [ ] 定义 `CpuInfo` 结构体
  - [ ] total_cores: 总核心数
  - [ ] physical_cores: 物理核心数
  - [ ] logical_cores: 逻辑核心数
  - [ ] available_cores: 可用核心列表
  - [ ] model_name: CPU 型号
  - [ ] frequency_mhz: 频率
- [ ] 实现 Linux `/proc/cpuinfo` 解析
- [ ] 实现 CPU 信息缓存
- [ ] 实现源文件 `src/core/cpu_affinity_manager.cpp`

### Acceptance Criteria
- 正确识别 CPU 核心数
- 区分物理核心和逻辑核心

---

## Task 2.2: CPU 亲和性管理器

**Status**: Not Started
**File**: `include/vectordb_bench/core/cpu_affinity_manager.h`

### Checklist
- [ ] 定义 `CpuConfig` 结构体（如未在 Phase 1 定义）
- [ ] 实现 `CpuAffinityManager` 类
  - [ ] GetCpuInfo(): 获取 CPU 信息
  - [ ] SetProcessAffinity(): 设置进程亲和性
  - [ ] SetThreadAffinity(): 设置线程亲和性
  - [ ] GetAvailableCores(): 获取可用核心
  - [ ] ResetAffinity(): 重置亲和性
- [ ] 实现 Linux sched_setaffinity 封装
- [ ] 实现 pthread_setaffinity_np 封装
- [ ] 实现核心分配策略

### Implementation Details

```cpp
// Linux implementation
bool CpuAffinityManager::SetLinuxAffinity(pid_t pid, 
                                          const std::vector<int>& core_ids) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  
  for (int core_id : core_ids) {
    CPU_SET(core_id, &cpuset);
  }
  
  return sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset) == 0;
}
```

### Acceptance Criteria
- 进程可以绑定到指定核心
- 线程可以绑定到指定核心
- 支持 -1 表示使用所有核心

---

## Task 2.3: 线程池实现

**Status**: Not Started
**File**: `include/vectordb_bench/core/thread_pool.h`

### Checklist
- [ ] 定义 `ThreadPool` 类
  - [ ] 构造函数 (num_threads, core_ids)
  - [ ] Submit(): 提交任务
  - [ ] WaitAll(): 等待所有任务完成
  - [ ] Pause() / Resume(): 暂停/恢复
  - [ ] GetThreadCount(): 获取线程数
- [ ] 实现任务队列
- [ ] 实现工作线程
- [ ] 实现线程核心绑定
- [ ] 实现源文件 `src/core/thread_pool.cpp`

### Implementation Details

```cpp
class ThreadPool {
 public:
  explicit ThreadPool(int num_threads, 
                      const std::vector<int>& core_ids = {});
  
  template<typename F, typename... Args>
  auto Submit(F&& f, Args&&... args) 
      -> std::future<typename std::invoke_result_t<F, Args...>>;

  void WaitAll();
  void Pause();
  void Resume();
  int GetThreadCount() const;

 private:
  void WorkerThread(int thread_id, int core_id);
};
```

### Acceptance Criteria
- 线程池支持任意任务类型
- 线程可绑定到指定核心
- WaitAll() 正确等待所有任务

---

## Task 2.4: 单元测试

**Status**: Not Started

### Checklist
- [ ] 编写 CpuInfo 解析测试
- [ ] 编写 SetProcessAffinity 测试
- [ ] 编写 SetThreadAffinity 测试
- [ ] 编写 ThreadPool 基本功能测试
- [ ] 编写 ThreadPool 核心绑定测试

### Acceptance Criteria
- 测试覆盖核心功能
- 需要实际 CPU 环境验证

---

## Dependencies

- Phase 1 完成
- pthread 库

## Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Linux | Supported | Primary target |
| macOS | Partial | pthread only |
| Windows | Not Supported | Future work |

## Notes

- CPU 亲和性是性能测试的关键能力
- 需要实际硬件环境测试验证
