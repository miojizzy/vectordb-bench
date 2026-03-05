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

#include "hnswlib_wrapper.h"

#include <algorithm>
#include <stdexcept>

namespace naive {

HnswlibWrapperImpl::HnswlibWrapperImpl(MetricType metric_type, int dimension,
                                       size_t max_elements, int M,
                                       int ef_construction) {
  switch (metric_type) {
    case MetricType::kL2:
      space_ = std::make_unique<hnswlib::L2Space>(dimension);
      break;
    case MetricType::kInnerProduct:
      space_ = std::make_unique<hnswlib::InnerProductSpace>(dimension);
      break;
    default:
      throw std::runtime_error("Unknown metric type");
  }
  
  index_ = std::make_unique<hnswlib::HierarchicalNSW<float>>(
      space_.get(), max_elements, M, ef_construction);
}

void HnswlibWrapperImpl::AddPoint(const float* data, size_t id) {
  index_->addPoint(data, static_cast<hnswlib::labeltype>(id));
}

std::vector<int> HnswlibWrapperImpl::SearchKnn(const float* query, int k,
                                               int ef_search) {
  index_->setEf(ef_search);
  auto result = index_->searchKnn(query, k);
  
  std::vector<int> ids;
  while (!result.empty()) {
    ids.push_back(static_cast<int>(result.top().second));
    result.pop();
  }
  std::reverse(ids.begin(), ids.end());
  return ids;
}

void HnswlibWrapperImpl::SetEfSearch(int ef_search) {
  index_->setEf(ef_search);
}

size_t HnswlibWrapperImpl::GetIndexSize() const {
  // Approximate index size
  return 0;  // hnswlib doesn't provide memory usage API
}

}  // namespace naive
