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

#include <string>
#include <vector>

namespace naive {

// Distance metric type
enum class MetricType {
  kL2,           // Euclidean distance
  kInnerProduct  // Inner product (for angular/cosine)
};

// Dataset container
struct Dataset {
  std::vector<std::vector<float>> train_vectors;
  std::vector<std::vector<float>> test_vectors;
  std::vector<std::vector<int>> ground_truth;

  int dimension = 0;
  int train_size = 0;
  int test_size = 0;
  int k = 0;  // ground truth k value
  MetricType metric_type = MetricType::kL2;
  std::string name;
};

// Dataset loader for fvecs/ivecs format files
class DatasetLoader {
 public:
  // Generic loader (auto-detect by directory name format: name-dimension-distance)
  static Dataset Load(const std::string& data_dir);

 private:
  // Read .fvecs file (float vectors)
  static std::vector<std::vector<float>> ReadFvecs(
      const std::string& filepath);

  // Read .ivecs file (integer vectors)
  static std::vector<std::vector<int>> ReadIvecs(
      const std::string& filepath);

  // Parse directory name to extract dataset info (name-dimension-distance)
  static void ParseDirectoryName(const std::string& dirname,
                                  std::string& name,
                                  int& dimension,
                                  MetricType& metric_type);
};

}  // namespace naive
