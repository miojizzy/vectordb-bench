// Copyright 2026 VectorDBBench Authors
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "index_manager.h"

#include <chrono>
#include <iostream>

#include "hnswlib_wrapper.h"

namespace naive {
namespace internal {

// Static members initialization
std::unordered_map<IndexManager::IndexKey, 
                   std::unique_ptr<IndexWrapper>, 
                   IndexManager::IndexKeyHash> IndexManager::indexes_;
std::mutex IndexManager::mutex_;

IndexWrapper* IndexManager::BuildAndCacheIndex(
    IndexType type,
    const Dataset& dataset,
    int M,
    int ef_construction) {
  
  auto key = MakeKey(type, M, ef_construction);
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Check if already cached
  auto it = indexes_.find(key);
  if (it != indexes_.end()) {
    return it->second.get();
  }
  
  // Build new index
  auto start = std::chrono::high_resolution_clock::now();
  
  auto index = CreateIndexWrapper(
      type, dataset.metric_type, dataset.dimension,
      dataset.train_size, M, ef_construction);
  
  for (int i = 0; i < dataset.train_size; ++i) {
    index->AddPoint(dataset.train_vectors[i].data(), i);
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  std::cout << "    Built " << CliParser::IndexTypeToString(type)
            << " (M=" << M << ", ef_construction=" << ef_construction << ")"
            << " in " << duration.count() << " ms" << std::endl;
  
  // Cache the index
  auto* ptr = index.get();
  indexes_[key] = std::move(index);
  
  return ptr;
}

IndexWrapper* IndexManager::GetOrCreateIndex(
    IndexType type,
    const Dataset& dataset,
    int M,
    int ef_construction) {
  
  auto* cached = GetIndex(type, M, ef_construction);
  if (cached) {
    return cached;
  }
  
  return BuildAndCacheIndex(type, dataset, M, ef_construction);
}

void IndexManager::BuildAllIndexes(
    const Dataset& dataset,
    const CliConfig& config,
    bool parallel) {
  
  std::cout << "\nBuilding indexes..." << std::endl;
  auto build_start = std::chrono::high_resolution_clock::now();
  
  if (parallel) {
    // Build indexes in parallel using threads
    std::vector<std::thread> threads;
    
    for (auto type : config.index_types) {
      for (int M : config.M_values) {
        for (int ef_construction : config.ef_construction_values) {
          threads.emplace_back([type, M, ef_construction, &dataset]() {
            BuildAndCacheIndex(type, dataset, M, ef_construction);
          });
        }
      }
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
      thread.join();
    }
  } else {
    // Build indexes sequentially
    for (auto type : config.index_types) {
      for (int M : config.M_values) {
        for (int ef_construction : config.ef_construction_values) {
          BuildAndCacheIndex(type, dataset, M, ef_construction);
        }
      }
    }
  }
  
  auto build_end = std::chrono::high_resolution_clock::now();
  auto build_duration = std::chrono::duration_cast<std::chrono::seconds>(
      build_end - build_start);
  
  std::cout << "\nAll indexes built in " << build_duration.count() 
            << " seconds" << std::endl;
}

bool IndexManager::HasIndex(IndexType type, int M, int ef_construction) {
  std::lock_guard<std::mutex> lock(mutex_);
  return indexes_.find(MakeKey(type, M, ef_construction)) != indexes_.end();
}

IndexWrapper* IndexManager::GetIndex(IndexType type, int M, int ef_construction) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = indexes_.find(MakeKey(type, M, ef_construction));
  return it != indexes_.end() ? it->second.get() : nullptr;
}

void IndexManager::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  indexes_.clear();
}

}  // namespace internal
}  // namespace naive
