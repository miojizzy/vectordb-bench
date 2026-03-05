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

#include "index_wrapper.h"

#include <algorithm>
#include <set>

namespace naive {

double CalculateRecall(
    const std::vector<std::vector<int>>& predicted,
    const std::vector<std::vector<int>>& ground_truth,
    int k) {
  
  double total_recall = 0.0;
  int num_queries = static_cast<int>(predicted.size());
  
  for (int i = 0; i < num_queries; ++i) {
    int gt_size = static_cast<int>(ground_truth[i].size());
    int effective_k = std::min(k, gt_size);
    
    std::set<int> gt_set(ground_truth[i].begin(),
                         ground_truth[i].begin() + effective_k);
    
    int correct = 0;
    int pred_size = static_cast<int>(predicted[i].size());
    int actual_k = std::min(effective_k, pred_size);
    
    for (int j = 0; j < actual_k; ++j) {
      if (gt_set.count(predicted[i][j])) {
        ++correct;
      }
    }
    
    total_recall += static_cast<double>(correct) / effective_k;
  }
  
  return total_recall / num_queries;
}

}  // namespace naive
