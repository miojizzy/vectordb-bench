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

#include "dataset_loader.h"

#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace naive {

std::vector<std::vector<float>> DatasetLoader::ReadFvecs(
    const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + filepath);
  }

  std::vector<std::vector<float>> vectors;

  while (file) {
    int dim = 0;
    file.read(reinterpret_cast<char*>(&dim), sizeof(int));
    if (!file || dim <= 0) break;

    std::vector<float> vec(dim);
    file.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(float));
    if (!file) break;

    vectors.push_back(std::move(vec));
  }

  return vectors;
}

std::vector<std::vector<int>> DatasetLoader::ReadIvecs(
    const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + filepath);
  }

  std::vector<std::vector<int>> vectors;

  while (file) {
    int dim = 0;
    file.read(reinterpret_cast<char*>(&dim), sizeof(int));
    if (!file || dim <= 0) break;

    std::vector<int> vec(dim);
    file.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(int));
    if (!file) break;

    vectors.push_back(std::move(vec));
  }

  return vectors;
}

void DatasetLoader::ParseDirectoryName(const std::string& dirname,
                                        std::string& name,
                                        int& dimension,
                                        MetricType& metric_type) {
  // Directory format: name-dimension-distance-topk{topk}
  // e.g., sift-128-euclidean-topk100, glove-50-angular-topk100,
  //       synthetic-64-euclidean-topk50, gist-960-euclidean-topk100

  // Find first dash
  size_t first_dash = dirname.find('-');
  if (first_dash == std::string::npos) {
    throw std::runtime_error("Invalid directory name format: " + dirname);
  }

  // Find second dash (between dimension and distance)
  size_t second_dash = dirname.find('-', first_dash + 1);
  if (second_dash == std::string::npos) {
    throw std::runtime_error("Invalid directory name format: " + dirname);
  }

  // Extract name
  name = dirname.substr(0, first_dash);

  // Extract dimension
  std::string dim_str = dirname.substr(first_dash + 1,
                                        second_dash - first_dash - 1);
  dimension = std::stoi(dim_str);

  // Extract distance (everything between second dash and "-topk")
  size_t topk_pos = dirname.find("-topk", second_dash + 1);
  if (topk_pos == std::string::npos) {
    throw std::runtime_error("Invalid directory name format: " + dirname +
                             " (missing -topk suffix)");
  }

  std::string distance = dirname.substr(second_dash + 1,
                                         topk_pos - second_dash - 1);

  if (distance == "euclidean") {
    metric_type = MetricType::kL2;
  } else if (distance == "angular" || distance == "inner-product") {
    metric_type = MetricType::kInnerProduct;
  } else {
    throw std::runtime_error("Unknown distance metric: " + distance);
  }
}

Dataset DatasetLoader::Load(const std::string& data_dir) {
  std::filesystem::path path(data_dir);
  std::string dirname = path.filename().string();

  Dataset dataset;
  ParseDirectoryName(dirname, dataset.name, dataset.dimension,
                     dataset.metric_type);

  // Transform name to uppercase for display
  std::string display_name = dataset.name;
  std::transform(display_name.begin(), display_name.end(),
                 display_name.begin(), ::toupper);
  dataset.name = display_name;

  // Load train vectors
  dataset.train_vectors = ReadFvecs(data_dir + "/base.fvecs");
  dataset.train_size = static_cast<int>(dataset.train_vectors.size());

  // Load test vectors
  dataset.test_vectors = ReadFvecs(data_dir + "/query.fvecs");
  dataset.test_size = static_cast<int>(dataset.test_vectors.size());

  // Load ground truth
  dataset.ground_truth = ReadIvecs(data_dir + "/groundtruth.ivecs");
  dataset.k = dataset.ground_truth.empty() ? 0
            : static_cast<int>(dataset.ground_truth[0].size());

  return dataset;
}

}  // namespace naive
