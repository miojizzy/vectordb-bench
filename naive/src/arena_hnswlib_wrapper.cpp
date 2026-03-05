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

#include "arena_hnswlib_wrapper.h"
#include "hnswlib_wrapper.h"

#include <algorithm>

namespace naive {

// Template implementation
template<typename SpaceT>
ArenaHnswlibWrapperImpl<SpaceT>::ArenaHnswlibWrapperImpl(
    SpaceT space, size_t max_elements, int M, int ef_construction,
    arena_hnswlib::Layer0NeighborMode mode, const std::string& name)
    : space_(std::move(space)), name_(name) {
  index_ = std::make_unique<arena_hnswlib::HierarchicalNSW<float, SpaceT>>(
      space_, max_elements, M, ef_construction, 42, mode);
}

template<typename SpaceT>
void ArenaHnswlibWrapperImpl<SpaceT>::AddPoint(const float* data, size_t id) {
  index_->addPoint(data, static_cast<arena_hnswlib::LabelType>(id));
}

template<typename SpaceT>
std::vector<int> ArenaHnswlibWrapperImpl<SpaceT>::SearchKnn(
    const float* query, int k, int ef_search) {
  index_->setEfSearch(ef_search);
  auto result = index_->searchKnn(query, k);
  
  std::vector<int> ids;
  while (!result.empty()) {
    ids.push_back(static_cast<int>(result.top().second));
    result.pop();
  }
  std::reverse(ids.begin(), ids.end());
  return ids;
}

template<typename SpaceT>
void ArenaHnswlibWrapperImpl<SpaceT>::SetEfSearch(int ef_search) {
  index_->setEfSearch(ef_search);
}

template<typename SpaceT>
size_t ArenaHnswlibWrapperImpl<SpaceT>::GetIndexSize() const {
  // Approximate index size
  return 0;  // arena-hnswlib memory usage
}

// Explicit template instantiation
template class ArenaHnswlibWrapperImpl<arena_hnswlib::L2Space<float>>;
template class ArenaHnswlibWrapperImpl<arena_hnswlib::InnerProductSpace<float>>;

// Factory function implementation
std::unique_ptr<IndexWrapper> CreateArenaHnswlibWrapper(
    IndexType type,
    MetricType metric_type,
    int dimension,
    size_t max_elements,
    int M,
    int ef_construction) {
  
  arena_hnswlib::Layer0NeighborMode mode;
  std::string name;
  
  switch (type) {
    case IndexType::kArenaDoubleM:
      mode = arena_hnswlib::Layer0NeighborMode::kDoubleM;
      name = "arena-DoubleM";
      break;
    case IndexType::kArenaHeuristicOnly:
      mode = arena_hnswlib::Layer0NeighborMode::kHeuristicOnly;
      name = "arena-HeuristicOnly";
      break;
    case IndexType::kArenaHeuristicPlusClosest:
      mode = arena_hnswlib::Layer0NeighborMode::kHeuristicPlusClosest;
      name = "arena-HeuristicPlusClosest";
      break;
    default:
      return nullptr;
  }
  
  if (metric_type == MetricType::kL2) {
    arena_hnswlib::L2Space<float> space(dimension);
    using Impl = ArenaHnswlibWrapperImpl<arena_hnswlib::L2Space<float>>;
    return std::make_unique<IndexWrapperAdapter<Impl>>(
        std::move(space), max_elements, M, ef_construction, mode, name);
  } else {
    arena_hnswlib::InnerProductSpace<float> space(dimension);
    using Impl = ArenaHnswlibWrapperImpl<arena_hnswlib::InnerProductSpace<float>>;
    return std::make_unique<IndexWrapperAdapter<Impl>>(
        std::move(space), max_elements, M, ef_construction, mode, name);
  }
}

// Factory function for creating any index type
std::unique_ptr<IndexWrapper> CreateIndexWrapper(
    IndexType type,
    MetricType metric_type,
    int dimension,
    size_t max_elements,
    int M,
    int ef_construction) {
  
  switch (type) {
    case IndexType::kHnswlib:
      return std::make_unique<HnswlibWrapper>(
          metric_type, dimension, max_elements, M, ef_construction);
    
    case IndexType::kArenaDoubleM:
    case IndexType::kArenaHeuristicOnly:
    case IndexType::kArenaHeuristicPlusClosest:
      return CreateArenaHnswlibWrapper(
          type, metric_type, dimension, max_elements, M, ef_construction);
    
    default:
      return nullptr;
  }
}

}  // namespace naive
