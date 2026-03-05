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

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "cli_parser.h"
#include "dataset_loader.h"
#include "index_wrapper.h"

namespace naive {
namespace internal {

// Global index manager
class IndexManager {
 private:
  // Cache key: index_type + M + ef_construction
  struct IndexKey {
    IndexType type;
    int M;
    int ef_construction;
    
    bool operator==(const IndexKey& other) const {
      return type == other.type && 
             M == other.M && 
             ef_construction == other.ef_construction;
    }
  };
  
  // Hash function for IndexKey
  struct IndexKeyHash {
    size_t operator()(const IndexKey& k) const {
      return static_cast<size_t>(k.type) * 10000 + k.M * 1000 + k.ef_construction;
    }
  };
  
  static std::unordered_map<IndexKey, std::unique_ptr<IndexWrapper>, IndexKeyHash> indexes_;
  static std::mutex mutex_;
  
 public:
  // Build and cache an index
  static IndexWrapper* BuildAndCacheIndex(
      IndexType type,
      const Dataset& dataset,
      int M,
      int ef_construction);
  
  // Get or create index (auto-build if not exists)
  static IndexWrapper* GetOrCreateIndex(
      IndexType type,
      const Dataset& dataset,
      int M,
      int ef_construction);
  
  // Build all indexes for specified types and parameters
  static void BuildAllIndexes(
      const Dataset& dataset,
      const CliConfig& config,
      bool parallel = false);
  
  // Check if index exists
  static bool HasIndex(IndexType type, int M, int ef_construction);
  
  // Get cached index
  static IndexWrapper* GetIndex(IndexType type, int M, int ef_construction);
  
  // Clear all cached indexes
  static void Clear();
  
 private:
  static IndexKey MakeKey(IndexType type, int M, int ef_construction) {
    return IndexKey{type, M, ef_construction};
  }
};

}  // namespace internal
}  // namespace naive
