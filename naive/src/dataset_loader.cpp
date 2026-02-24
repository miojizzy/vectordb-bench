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

Dataset DatasetLoader::LoadSIFT(const std::string& data_dir) {
  Dataset dataset;
  dataset.name = "SIFT";
  dataset.metric_type = MetricType::kL2;

  // Load train vectors
  dataset.train_vectors = ReadFvecs(data_dir + "/sift_base.fvecs");
  dataset.train_size = static_cast<int>(dataset.train_vectors.size());
  dataset.dimension = dataset.train_vectors.empty() ? 0
                     : static_cast<int>(dataset.train_vectors[0].size());

  // Load test vectors
  dataset.test_vectors = ReadFvecs(data_dir + "/sift_query.fvecs");
  dataset.test_size = static_cast<int>(dataset.test_vectors.size());

  // Load ground truth
  dataset.ground_truth = ReadIvecs(data_dir + "/sift_groundtruth.ivecs");
  dataset.k = dataset.ground_truth.empty() ? 0
            : static_cast<int>(dataset.ground_truth[0].size());

  return dataset;
}

Dataset DatasetLoader::LoadGloVe(const std::string& data_dir) {
  Dataset dataset;
  dataset.name = "GloVe-50";
  dataset.metric_type = MetricType::kInnerProduct;

  // Load train vectors
  dataset.train_vectors = ReadFvecs(data_dir + "/train.fvecs");
  dataset.train_size = static_cast<int>(dataset.train_vectors.size());
  dataset.dimension = dataset.train_vectors.empty() ? 0
                     : static_cast<int>(dataset.train_vectors[0].size());

  // Load test vectors
  dataset.test_vectors = ReadFvecs(data_dir + "/test.fvecs");
  dataset.test_size = static_cast<int>(dataset.test_vectors.size());

  // Load ground truth
  dataset.ground_truth = ReadIvecs(data_dir + "/groundtruth.ivecs");
  dataset.k = dataset.ground_truth.empty() ? 0
            : static_cast<int>(dataset.ground_truth[0].size());

  return dataset;
}

Dataset DatasetLoader::LoadFashionMNIST(const std::string& data_dir) {
  Dataset dataset;
  dataset.name = "Fashion-MNIST";
  dataset.metric_type = MetricType::kL2;

  dataset.train_vectors = ReadFvecs(data_dir + "/sift_base.fvecs");
  dataset.train_size = static_cast<int>(dataset.train_vectors.size());
  dataset.dimension = dataset.train_vectors.empty() ? 0
                     : static_cast<int>(dataset.train_vectors[0].size());

  dataset.test_vectors = ReadFvecs(data_dir + "/sift_query.fvecs");
  dataset.test_size = static_cast<int>(dataset.test_vectors.size());

  dataset.ground_truth = ReadIvecs(data_dir + "/sift_groundtruth.ivecs");
  dataset.k = dataset.ground_truth.empty() ? 0
            : static_cast<int>(dataset.ground_truth[0].size());

  return dataset;
}

Dataset DatasetLoader::LoadLastfm(const std::string& data_dir) {
  Dataset dataset;
  dataset.name = "Last.fm";
  dataset.metric_type = MetricType::kInnerProduct;

  dataset.train_vectors = ReadFvecs(data_dir + "/train.fvecs");
  dataset.train_size = static_cast<int>(dataset.train_vectors.size());
  dataset.dimension = dataset.train_vectors.empty() ? 0
                     : static_cast<int>(dataset.train_vectors[0].size());

  dataset.test_vectors = ReadFvecs(data_dir + "/test.fvecs");
  dataset.test_size = static_cast<int>(dataset.test_vectors.size());

  dataset.ground_truth = ReadIvecs(data_dir + "/groundtruth.ivecs");
  dataset.k = dataset.ground_truth.empty() ? 0
            : static_cast<int>(dataset.ground_truth[0].size());

  return dataset;
}

Dataset DatasetLoader::Load(const std::string& data_dir) {
  std::filesystem::path path(data_dir);
  std::string dirname = path.filename().string();

  if (dirname.find("sift") != std::string::npos) {
    return LoadSIFT(data_dir);
  } else if (dirname.find("glove") != std::string::npos) {
    return LoadGloVe(data_dir);
  } else if (dirname.find("fashion") != std::string::npos ||
             dirname.find("mnist") != std::string::npos) {
    return LoadFashionMNIST(data_dir);
  } else if (dirname.find("lastfm") != std::string::npos ||
             dirname.find("last.fm") != std::string::npos) {
    return LoadLastfm(data_dir);
  } else {
    throw std::runtime_error("Unknown dataset: " + dirname);
  }
}

}  // namespace naive
