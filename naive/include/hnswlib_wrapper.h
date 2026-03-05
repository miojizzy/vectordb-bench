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

#include <hnswlib/hnswlib.h>
#include <memory>

#include "index_wrapper.h"

namespace naive {

// hnswlib implementation (can be inlined)
class HnswlibWrapperImpl {
 private:
  std::unique_ptr<hnswlib::SpaceInterface<float>> space_;
  std::unique_ptr<hnswlib::HierarchicalNSW<float>> index_;
  
 public:
  HnswlibWrapperImpl(MetricType metric_type, int dimension,
                     size_t max_elements, int M, int ef_construction);
  
  void AddPoint(const float* data, size_t id);
  std::vector<int> SearchKnn(const float* query, int k, int ef_search);
  void SetEfSearch(int ef_search);
  const char* GetName() const { return "hnswlib"; }
  size_t GetIndexSize() const;
};

// Adapter to connect impl to IndexWrapper interface
template<typename ImplT>
class IndexWrapperAdapter : public IndexWrapper {
 private:
  ImplT impl_;
  
 public:
  template<typename... Args>
  explicit IndexWrapperAdapter(Args&&... args) 
      : impl_(std::forward<Args>(args)...) {}
  
  void AddPoint(const float* data, size_t id) override {
    impl_.AddPoint(data, id);
  }
  
  std::vector<int> SearchKnn(const float* query, int k, int ef_search) override {
    return impl_.SearchKnn(query, k, ef_search);
  }
  
  void SetEfSearch(int ef_search) override {
    impl_.SetEfSearch(ef_search);
  }
  
  const char* GetName() const override {
    return impl_.GetName();
  }
  
  size_t GetIndexSize() const override {
    return impl_.GetIndexSize();
  }
};

// Type alias for hnswlib wrapper
using HnswlibWrapper = IndexWrapperAdapter<HnswlibWrapperImpl>;

}  // namespace naive
