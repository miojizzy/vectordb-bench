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

#include <gtest/gtest.h>
#include "arena_hnswlib_wrapper.h"

namespace naive {
namespace {

// 测试不同 Layer0NeighborMode 的构造
TEST(ArenaHnswlibWrapperTest, Constructor) {
  const int kDimension = 64;
  const int kMaxElements = 1000;
  const int kM = 16;
  const int kEfConstruction = 200;
  
  // 测试 DoubleM 模式
  EXPECT_NO_THROW({
    auto wrapper = CreateArenaHnswlibWrapper(
        IndexType::kArenaDoubleM,
        MetricType::kL2,
        kDimension,
        kMaxElements,
        kM,
        kEfConstruction
    );
    EXPECT_NE(wrapper, nullptr);
  });
  
  // 测试 HeuristicOnly 模式
  EXPECT_NO_THROW({
    auto wrapper = CreateArenaHnswlibWrapper(
        IndexType::kArenaHeuristicOnly,
        MetricType::kL2,
        kDimension,
        kMaxElements,
        kM,
        kEfConstruction
    );
    EXPECT_NE(wrapper, nullptr);
  });
  
  // 测试 HeuristicPlusClosest 模式
  EXPECT_NO_THROW({
    auto wrapper = CreateArenaHnswlibWrapper(
        IndexType::kArenaHeuristicPlusClosest,
        MetricType::kL2,
        kDimension,
        kMaxElements,
        kM,
        kEfConstruction
    );
    EXPECT_NE(wrapper, nullptr);
  });
  
  // 测试内积距离
  EXPECT_NO_THROW({
    auto wrapper = CreateArenaHnswlibWrapper(
        IndexType::kArenaDoubleM,
        MetricType::kInnerProduct,
        kDimension,
        kMaxElements,
        kM,
        kEfConstruction
    );
    EXPECT_NE(wrapper, nullptr);
  });
}

// 测试添加点功能
TEST(ArenaHnswlibWrapperTest, AddPoint) {
  const int kDimension = 32;
  const int kMaxElements = 100;
  
  auto wrapper = CreateArenaHnswlibWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kL2,
      kDimension,
      kMaxElements,
      16,
      100
  );
  
  ASSERT_NE(wrapper, nullptr);
  
  // 添加单个点
  std::vector<float> vec1(kDimension, 1.0f);
  EXPECT_NO_THROW(wrapper->AddPoint(vec1.data(), 0));
  
  // 添加多个点
  for (int i = 1; i < 50; ++i) {
    std::vector<float> vec(kDimension, static_cast<float>(i));
    EXPECT_NO_THROW(wrapper->AddPoint(vec.data(), i));
  }
  
  // 注意：GetIndexSize() 返回 0，因为库不提供内存使用 API
  // 通过搜索功能验证添加成功
  std::vector<int> results = wrapper->SearchKnn(vec1.data(), 5, 100);
  EXPECT_EQ(results.size(), 5);
}

// 测试搜索功能
TEST(ArenaHnswlibWrapperTest, SearchKnn) {
  const int kDimension = 32;
  const int kMaxElements = 100;
  
  auto wrapper = CreateArenaHnswlibWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kL2,
      kDimension,
      kMaxElements,
      16,
      100
  );
  
  ASSERT_NE(wrapper, nullptr);
  
  // 添加一些已知向量
  std::vector<std::vector<float>> vectors;
  for (int i = 0; i < 20; ++i) {
    std::vector<float> vec(kDimension, static_cast<float>(i));
    vectors.push_back(vec);
    wrapper->AddPoint(vec.data(), i);
  }
  
  // 用第一个向量搜索
  int k = 5;
  int ef_search = 50;
  std::vector<int> results = wrapper->SearchKnn(vectors[0].data(), k, ef_search);
  
  EXPECT_EQ(results.size(), static_cast<size_t>(k));
  
  // 最近邻应该是自己
  EXPECT_EQ(results[0], 0);
}

// 测试不同模式下的搜索质量
TEST(ArenaHnswlibWrapperTest, DifferentModes) {
  const int kDimension = 16;
  const int kMaxElements = 100;
  const int kM = 8;
  const int kEfConstruction = 100;
  
  std::vector<IndexType> modes = {
    IndexType::kArenaDoubleM,
    IndexType::kArenaHeuristicOnly,
    IndexType::kArenaHeuristicPlusClosest
  };
  
  // 准备相同的测试数据
  std::vector<std::vector<float>> vectors;
  for (int i = 0; i < 30; ++i) {
    std::vector<float> vec(kDimension);
    for (int j = 0; j < kDimension; ++j) {
      vec[j] = static_cast<float>(i + j * 0.1);
    }
    vectors.push_back(vec);
  }
  
  for (auto type : modes) {
    auto wrapper = CreateArenaHnswlibWrapper(
        type,
        MetricType::kL2,
        kDimension,
        kMaxElements,
        kM,
        kEfConstruction
    );
    
    ASSERT_NE(wrapper, nullptr) << "Failed to create wrapper for type: "
                                << static_cast<int>(type);
    
    // 添加向量
    for (size_t i = 0; i < vectors.size(); ++i) {
      wrapper->AddPoint(vectors[i].data(), i);
    }
    
    // 搜索
    int k = 10;
    int ef_search = 50;
    auto results = wrapper->SearchKnn(vectors[0].data(), k, ef_search);
    
    EXPECT_EQ(results.size(), static_cast<size_t>(k))
        << "Wrong result size for type: " << static_cast<int>(type);
    
    // 检查最近邻是否是自己
    EXPECT_EQ(results[0], 0)
        << "Wrong nearest neighbor for type: " << static_cast<int>(type);
  }
}

// 测试 SetEfSearch 功能
TEST(ArenaHnswlibWrapperTest, SetEfSearch) {
  const int kDimension = 32;
  const int kMaxElements = 100;
  
  auto wrapper = CreateArenaHnswlibWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kL2,
      kDimension,
      kMaxElements,
      16,
      100
  );
  
  ASSERT_NE(wrapper, nullptr);
  
  // 添加一些向量
  for (int i = 0; i < 30; ++i) {
    std::vector<float> vec(kDimension, static_cast<float>(i));
    wrapper->AddPoint(vec.data(), i);
  }
  
  // 测试不同的 ef_search 值
  std::vector<float> query(kDimension, 0.0f);
  
  wrapper->SetEfSearch(20);
  auto results1 = wrapper->SearchKnn(query.data(), 10, 20);
  
  wrapper->SetEfSearch(100);
  auto results2 = wrapper->SearchKnn(query.data(), 10, 100);
  
  EXPECT_EQ(results1.size(), 10);
  EXPECT_EQ(results2.size(), 10);
}

// 测试 GetName
TEST(ArenaHnswlibWrapperTest, GetName) {
  auto wrapper_doublem = CreateArenaHnswlibWrapper(
      IndexType::kArenaDoubleM, MetricType::kL2, 16, 100, 16, 100);
  ASSERT_NE(wrapper_doublem, nullptr);
  EXPECT_STREQ(wrapper_doublem->GetName(), "arena-DoubleM");
  
  auto wrapper_heuristic = CreateArenaHnswlibWrapper(
      IndexType::kArenaHeuristicOnly, MetricType::kL2, 16, 100, 16, 100);
  ASSERT_NE(wrapper_heuristic, nullptr);
  EXPECT_STREQ(wrapper_heuristic->GetName(), "arena-HeuristicOnly");
  
  auto wrapper_plus = CreateArenaHnswlibWrapper(
      IndexType::kArenaHeuristicPlusClosest, MetricType::kL2, 16, 100, 16, 100);
  ASSERT_NE(wrapper_plus, nullptr);
  EXPECT_STREQ(wrapper_plus->GetName(), "arena-HeuristicPlusClosest");
}

// 测试内积距离
TEST(ArenaHnswlibWrapperTest, InnerProductDistance) {
  const int kDimension = 16;
  const int kMaxElements = 50;
  
  auto wrapper = CreateArenaHnswlibWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kInnerProduct,
      kDimension,
      kMaxElements,
      16,
      100
  );
  
  ASSERT_NE(wrapper, nullptr);
  
  // 添加归一化向量
  std::vector<float> vec1(kDimension, 1.0f / std::sqrt(kDimension));
  std::vector<float> vec2(kDimension, 2.0f / std::sqrt(kDimension));
  
  wrapper->AddPoint(vec1.data(), 0);
  wrapper->AddPoint(vec2.data(), 1);
  
  // 搜索最相似的向量
  auto results = wrapper->SearchKnn(vec1.data(), 2, 50);
  EXPECT_EQ(results.size(), 2);
  
  // 内积距离：vec1 和 vec2 的内积(=2.0) 大于 vec1 和 vec1 的内积(=1.0)
  // 所以 vec2 (id=1) 应该是最近邻
  EXPECT_EQ(results[0], 1);
  EXPECT_EQ(results[1], 0);
}

// 测试边界情况
TEST(ArenaHnswlibWrapperTest, EdgeCases) {
  const int kDimension = 16;
  const int kMaxElements = 10;
  
  auto wrapper = CreateArenaHnswlibWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kL2,
      kDimension,
      kMaxElements,
      8,
      50
  );
  
  ASSERT_NE(wrapper, nullptr);
  
  // 添加到最大容量
  for (int i = 0; i < kMaxElements; ++i) {
    std::vector<float> vec(kDimension, static_cast<float>(i));
    wrapper->AddPoint(vec.data(), i);
  }
  
  // 搜索 k 大于索引大小
  std::vector<float> query(kDimension, 0.0f);
  auto results = wrapper->SearchKnn(query.data(), kMaxElements, 50);
  EXPECT_LE(results.size(), static_cast<size_t>(kMaxElements));
}

// 性能对比测试（不同模式）
TEST(ArenaHnswlibWrapperTest, PerformanceComparison) {
  const int kDimension = 64;
  const int kMaxElements = 100;
  const int kNumQueries = 10;
  
  std::vector<IndexType> modes = {
    IndexType::kArenaDoubleM,
    IndexType::kArenaHeuristicOnly,
    IndexType::kArenaHeuristicPlusClosest
  };
  
  // 准备测试数据
  std::vector<std::vector<float>> vectors;
  for (int i = 0; i < kMaxElements; ++i) {
    std::vector<float> vec(kDimension);
    for (int j = 0; j < kDimension; ++j) {
      vec[j] = static_cast<float>(rand()) / RAND_MAX;
    }
    vectors.push_back(vec);
  }
  
  std::vector<std::vector<float>> queries;
  for (int i = 0; i < kNumQueries; ++i) {
    std::vector<float> query(kDimension);
    for (int j = 0; j < kDimension; ++j) {
      query[j] = static_cast<float>(rand()) / RAND_MAX;
    }
    queries.push_back(query);
  }
  
  for (auto type : modes) {
    auto wrapper = CreateArenaHnswlibWrapper(
        type,
        MetricType::kL2,
        kDimension,
        kMaxElements,
        16,
        200
    );
    
    ASSERT_NE(wrapper, nullptr);
    
    // 构建索引
    for (size_t i = 0; i < vectors.size(); ++i) {
      wrapper->AddPoint(vectors[i].data(), i);
    }
    
    // 执行查询
    for (const auto& query : queries) {
      auto results = wrapper->SearchKnn(query.data(), 10, 100);
      EXPECT_EQ(results.size(), 10);
    }
  }
}

}  // namespace
}  // namespace naive
