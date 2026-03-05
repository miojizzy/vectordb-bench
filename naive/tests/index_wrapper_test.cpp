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
#include "index_wrapper.h"
#include "hnswlib_wrapper.h"
#include "arena_hnswlib_wrapper.h"

namespace naive {
namespace {

// 测试索引包装器工厂函数
TEST(IndexWrapperTest, CreateIndexWrapper) {
  // 测试创建 hnswlib 索引
  auto hnswlib_index = CreateIndexWrapper(
      IndexType::kHnswlib,
      MetricType::kL2,
      128,   // dimension
      1000,  // max_elements
      16,    // M
      200    // ef_construction
  );
  
  EXPECT_NE(hnswlib_index, nullptr);
  EXPECT_STREQ(hnswlib_index->GetName(), "hnswlib");
  
  // 测试创建 arena_doublem 索引
  auto arena_index = CreateIndexWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kL2,
      128,
      1000,
      16,
      200
  );
  
  EXPECT_NE(arena_index, nullptr);
  // 名称格式为 "arena-DoubleM"
  EXPECT_STREQ(arena_index->GetName(), "arena-DoubleM");
}

// 测试 hnswlib 包装器的基本功能
TEST(IndexWrapperTest, HnswlibBasicOperations) {
  const int kDimension = 16;
  const int kMaxElements = 100;
  const int kM = 8;
  const int kEfConstruction = 100;
  
  auto index = CreateIndexWrapper(
      IndexType::kHnswlib,
      MetricType::kL2,
      kDimension,
      kMaxElements,
      kM,
      kEfConstruction
  );
  
  ASSERT_NE(index, nullptr);
  
  // 添加一些测试向量
  std::vector<std::vector<float>> vectors;
  for (int i = 0; i < 50; ++i) {
    std::vector<float> vec(kDimension);
    for (int j = 0; j < kDimension; ++j) {
      vec[j] = static_cast<float>(i + j * 0.1);
    }
    vectors.push_back(vec);
    index->AddPoint(vec.data(), i);
  }
  
  // 注意：GetIndexSize() 返回 0，因为 hnswlib 不提供内存使用 API
  // 只测试搜索功能
  
  // 测试搜索功能
  std::vector<int> results = index->SearchKnn(vectors[0].data(), 5, 100);
  EXPECT_EQ(results.size(), 5);
  
  // 第一个结果应该是查询向量本身（id=0）
  EXPECT_EQ(results[0], 0);
}

// 测试 arena-hnswlib 包装器的基本功能
TEST(IndexWrapperTest, ArenaHnswlibBasicOperations) {
  const int kDimension = 16;
  const int kMaxElements = 100;
  const int kM = 8;
  const int kEfConstruction = 100;
  
  auto index = CreateIndexWrapper(
      IndexType::kArenaDoubleM,
      MetricType::kL2,
      kDimension,
      kMaxElements,
      kM,
      kEfConstruction
  );
  
  ASSERT_NE(index, nullptr);
  
  // 添加一些测试向量
  std::vector<std::vector<float>> vectors;
  for (int i = 0; i < 50; ++i) {
    std::vector<float> vec(kDimension);
    for (int j = 0; j < kDimension; ++j) {
      vec[j] = static_cast<float>(i + j * 0.1);
    }
    vectors.push_back(vec);
    index->AddPoint(vec.data(), i);
  }
  
  // 注意：GetIndexSize() 返回 0，因为库不提供内存使用 API
  // 只测试搜索功能
  
  // 测试搜索功能
  std::vector<int> results = index->SearchKnn(vectors[0].data(), 5, 100);
  EXPECT_EQ(results.size(), 5);
  
  // 第一个结果应该是查询向量本身（id=0）
  EXPECT_EQ(results[0], 0);
}

// 测试内积距离
TEST(IndexWrapperTest, InnerProductMetric) {
  const int kDimension = 16;
  const int kMaxElements = 100;
  
  auto index = CreateIndexWrapper(
      IndexType::kHnswlib,
      MetricType::kInnerProduct,
      kDimension,
      kMaxElements,
      16,
      200
  );
  
  ASSERT_NE(index, nullptr);
  
  // 归一化向量
  std::vector<float> vec1(kDimension, 1.0f / std::sqrt(kDimension));
  std::vector<float> vec2(kDimension, 1.0f / std::sqrt(kDimension));
  
  index->AddPoint(vec1.data(), 0);
  index->AddPoint(vec2.data(), 1);
  
  // 搜索最相似的向量
  std::vector<int> results = index->SearchKnn(vec1.data(), 2, 100);
  EXPECT_EQ(results.size(), 2);
}

// 测试 SetEfSearch
TEST(IndexWrapperTest, SetEfSearch) {
  auto index = CreateIndexWrapper(
      IndexType::kHnswlib,
      MetricType::kL2,
      16, 100, 16, 200
  );
  
  ASSERT_NE(index, nullptr);
  
  // 添加一些向量
  for (int i = 0; i < 20; ++i) {
    std::vector<float> vec(16, static_cast<float>(i));
    index->AddPoint(vec.data(), i);
  }
  
  // 测试不同的 ef_search 值
  std::vector<float> query(16, 0.0f);
  
  index->SetEfSearch(10);
  auto results1 = index->SearchKnn(query.data(), 5, 10);
  
  index->SetEfSearch(100);
  auto results2 = index->SearchKnn(query.data(), 5, 100);
  
  // 两种设置都应该返回 k 个结果
  EXPECT_EQ(results1.size(), 5);
  EXPECT_EQ(results2.size(), 5);
}

// 测试所有索引类型
TEST(IndexWrapperTest, AllIndexTypes) {
  const std::vector<IndexType> types = {
    IndexType::kHnswlib,
    IndexType::kArenaDoubleM,
    IndexType::kArenaHeuristicOnly,
    IndexType::kArenaHeuristicPlusClosest
  };
  
  for (auto type : types) {
    auto index = CreateIndexWrapper(type, MetricType::kL2, 16, 100, 16, 200);
    ASSERT_NE(index, nullptr) << "Failed to create index for type: "
                              << static_cast<int>(type);
    
    // 添加一些向量
    for (int i = 0; i < 10; ++i) {
      std::vector<float> vec(16, static_cast<float>(i));
      index->AddPoint(vec.data(), i);
    }
    
    // 搜索
    std::vector<float> query(16, 0.0f);
    auto results = index->SearchKnn(query.data(), 3, 50);
    EXPECT_EQ(results.size(), 3) << "Search failed for type: "
                                 << static_cast<int>(type);
  }
}

}  // namespace
}  // namespace naive
