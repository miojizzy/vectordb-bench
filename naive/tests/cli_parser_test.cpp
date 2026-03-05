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
#include "cli_parser.h"

namespace naive {
namespace {

// 测试 IndexType 转换函数
TEST(CliParserTest, IndexTypeToString) {
  EXPECT_EQ(CliParser::IndexTypeToString(IndexType::kHnswlib), "hnswlib");
  EXPECT_EQ(CliParser::IndexTypeToString(IndexType::kArenaDoubleM), 
            "arena-DoubleM");
  EXPECT_EQ(CliParser::IndexTypeToString(IndexType::kArenaHeuristicOnly), 
            "arena-HeuristicOnly");
  EXPECT_EQ(CliParser::IndexTypeToString(IndexType::kArenaHeuristicPlusClosest),
            "arena-HeuristicPlusClosest");
}

// 测试字符串转 IndexType
TEST(CliParserTest, ParseIndexType) {
  EXPECT_EQ(CliParser::ParseIndexType("hnswlib"), IndexType::kHnswlib);
  EXPECT_EQ(CliParser::ParseIndexType("doublem"), IndexType::kArenaDoubleM);
  EXPECT_EQ(CliParser::ParseIndexType("arena_doublem"), 
            IndexType::kArenaDoubleM);
  EXPECT_EQ(CliParser::ParseIndexType("heuristiconly"),
            IndexType::kArenaHeuristicOnly);
  EXPECT_EQ(CliParser::ParseIndexType("arena_heuristiconly"),
            IndexType::kArenaHeuristicOnly);
  EXPECT_EQ(CliParser::ParseIndexType("heuristicplusclosest"),
            IndexType::kArenaHeuristicPlusClosest);
  EXPECT_EQ(CliParser::ParseIndexType("arena_heuristicplusclosest"),
            IndexType::kArenaHeuristicPlusClosest);
  
  // 测试不区分大小写
  EXPECT_EQ(CliParser::ParseIndexType("HNSWLIB"), IndexType::kHnswlib);
  EXPECT_EQ(CliParser::ParseIndexType("DOUBLEM"), IndexType::kArenaDoubleM);
  
  // 测试无效类型
  EXPECT_THROW(CliParser::ParseIndexType("invalid"), std::invalid_argument);
}

// 测试基本参数解析
TEST(CliParserTest, ParseBasicArgs) {
  const char* argv[] = {
    "test_program",
    "--data_dir", "/path/to/data",
    "--output_file", "results.txt"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  CliConfig config = CliParser::Parse(argc, const_cast<char**>(argv));
  
  EXPECT_EQ(config.data_dir, "/path/to/data");
  EXPECT_EQ(config.output_file, "results.txt");
  
  // 检查默认值（M_values 和 ef_construction_values 有默认值）
  EXPECT_EQ(config.M_values.size(), 1);
  EXPECT_EQ(config.M_values[0], 16);
  EXPECT_EQ(config.ef_construction_values.size(), 1);
  EXPECT_EQ(config.ef_construction_values[0], 200);
  EXPECT_EQ(config.k, 100);
  EXPECT_FALSE(config.build_index_parallel);
}

// 测试索引类型参数解析
TEST(CliParserTest, ParseIndexTypes) {
  const char* argv[] = {
    "test_program",
    "--data_dir", "/data",
    "--index_types", "hnswlib,arena_doublem"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  CliConfig config = CliParser::Parse(argc, const_cast<char**>(argv));
  
  EXPECT_EQ(config.index_types.size(), 2);
  EXPECT_EQ(config.index_types[0], IndexType::kHnswlib);
  EXPECT_EQ(config.index_types[1], IndexType::kArenaDoubleM);
}

// 测试数值参数解析
TEST(CliParserTest, ParseNumericArgs) {
  const char* argv[] = {
    "test_program",
    "--data_dir", "/data",
    "--M", "32",
    "--ef_construction", "400",
    "--k", "50"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  CliConfig config = CliParser::Parse(argc, const_cast<char**>(argv));
  
  ASSERT_EQ(config.M_values.size(), 1);
  EXPECT_EQ(config.M_values[0], 32);
  ASSERT_EQ(config.ef_construction_values.size(), 1);
  EXPECT_EQ(config.ef_construction_values[0], 400);
  EXPECT_EQ(config.k, 50);
}

// 测试列表参数解析
TEST(CliParserTest, ParseListArgs) {
  const char* argv[] = {
    "test_program",
    "--data_dir", "/data",
    "--ef_search", "100,200,500",
    "--num_threads", "1,2,4,8"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  CliConfig config = CliParser::Parse(argc, const_cast<char**>(argv));
  
  ASSERT_EQ(config.ef_search_values.size(), 3);
  EXPECT_EQ(config.ef_search_values[0], 100);
  EXPECT_EQ(config.ef_search_values[1], 200);
  EXPECT_EQ(config.ef_search_values[2], 500);
  
  ASSERT_EQ(config.num_threads_values.size(), 4);
  EXPECT_EQ(config.num_threads_values[0], 1);
  EXPECT_EQ(config.num_threads_values[1], 2);
  EXPECT_EQ(config.num_threads_values[2], 4);
  EXPECT_EQ(config.num_threads_values[3], 8);
}

// 测试布尔参数解析
TEST(CliParserTest, ParseBoolArgs) {
  const char* argv[] = {
    "test_program",
    "--data_dir", "/data",
    "--build_parallel"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  CliConfig config = CliParser::Parse(argc, const_cast<char**>(argv));
  
  EXPECT_TRUE(config.build_index_parallel);
}

// 测试 Google Benchmark 参数透传（使用 = 格式）
TEST(CliParserTest, ParseBenchmarkArgs) {
  const char* argv[] = {
    "test_program",
    "--data_dir", "/data",
    "--benchmark_filter=BM_Search",
    "--benchmark_format=json",
    "--benchmark_out=results.json"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  CliConfig config = CliParser::Parse(argc, const_cast<char**>(argv));
  
  EXPECT_EQ(config.benchmark_filter, "BM_Search");
  EXPECT_EQ(config.benchmark_format, "json");
  EXPECT_EQ(config.benchmark_out, "results.json");
}

// 测试缺少必需参数
TEST(CliParserTest, MissingRequiredArgs) {
  const char* argv[] = {"test_program"};
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  EXPECT_THROW(CliParser::Parse(argc, const_cast<char**>(argv)),
               std::invalid_argument);
}

// 测试帮助参数
TEST(CliParserTest, HelpArg) {
  const char* argv[] = {"test_program", "--help"};
  int argc = sizeof(argv) / sizeof(argv[0]);
  
  // 帮助参数会调用 exit(0)，所以这里只测试不会抛出异常
  // 在实际测试中，可以重定向 stdout 来验证输出
  EXPECT_EXIT(CliParser::Parse(argc, const_cast<char**>(argv)),
              ::testing::ExitedWithCode(0), ".*");
}

}  // namespace
}  // namespace naive
