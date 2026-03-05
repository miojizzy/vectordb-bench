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
#include <vector>

#include "cli_parser.h"
#include "dataset_loader.h"

namespace naive {

// Abstract index wrapper - Type erasure interface
class IndexWrapper {
 public:
  virtual ~IndexWrapper() = default;
  
  // Add a point to the index
  virtual void AddPoint(const float* data, size_t id) = 0;
  
  // Search for k nearest neighbors
  virtual std::vector<int> SearchKnn(const float* query, int k, int ef_search) = 0;
  
  // Set search ef parameter
  virtual void SetEfSearch(int ef_search) = 0;
  
  // Get index name
  virtual const char* GetName() const = 0;
  
  // Get index size in bytes (optional, can return 0 if not supported)
  virtual size_t GetIndexSize() const { return 0; }
};

// Factory function to create index wrapper
std::unique_ptr<IndexWrapper> CreateIndexWrapper(
    IndexType type,
    MetricType metric_type,
    int dimension,
    size_t max_elements,
    int M,
    int ef_construction);

// Calculate recall@k
double CalculateRecall(
    const std::vector<std::vector<int>>& predicted,
    const std::vector<std::vector<int>>& ground_truth,
    int k);

}  // namespace naive
