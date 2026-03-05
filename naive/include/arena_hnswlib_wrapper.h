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

#include <arena-hnswlib/arena_hnswlib.h>
#include <arena-hnswlib/hnswalg.h>
#include <arena-hnswlib/space_l2.h>
#include <arena-hnswlib/space_ip.h>
#include <memory>
#include <string>

#include "index_wrapper.h"

namespace naive {

// arena-hnswlib implementation (template-based, can be inlined)
template<typename SpaceT>
class ArenaHnswlibWrapperImpl {
 private:
  SpaceT space_;
  std::unique_ptr<arena_hnswlib::HierarchicalNSW<float, SpaceT>> index_;
  std::string name_;
  
 public:
  ArenaHnswlibWrapperImpl(SpaceT space, size_t max_elements,
                          int M, int ef_construction,
                          arena_hnswlib::Layer0NeighborMode mode,
                          const std::string& name);
  
  void AddPoint(const float* data, size_t id);
  std::vector<int> SearchKnn(const float* query, int k, int ef_search);
  void SetEfSearch(int ef_search);
  const char* GetName() const { return name_.c_str(); }
  size_t GetIndexSize() const;
};

// Factory function to create arena-hnswlib wrapper
std::unique_ptr<IndexWrapper> CreateArenaHnswlibWrapper(
    IndexType type,
    MetricType metric_type,
    int dimension,
    size_t max_elements,
    int M,
    int ef_construction);

}  // namespace naive
