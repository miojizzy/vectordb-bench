#!/usr/bin/env python3
# Copyright 2026 VectorDBBench Authors
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Generate test dataset for benchmark validation."""

import numpy as np
import struct
import os
import sys

def write_fvecs(filename, vectors):
    """Write vectors to fvecs format."""
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.float32).tobytes())

def write_ivecs(filename, vectors):
    """Write integer vectors to ivecs format."""
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.int32).tobytes())

def generate_sift_like_dataset(output_dir, num_train=10000, num_test=1000,
                               dim=128, k=100, seed=42):
    """Generate a SIFT-like dataset for testing."""
    np.random.seed(seed)
    
    # Generate random train vectors
    train_vectors = np.random.randn(num_train, dim).astype(np.float32)
    
    # Normalize to make them more realistic
    train_vectors = train_vectors / np.linalg.norm(train_vectors, axis=1, keepdims=True)
    
    # Generate test vectors (queries)
    test_vectors = np.random.randn(num_test, dim).astype(np.float32)
    test_vectors = test_vectors / np.linalg.norm(test_vectors, axis=1, keepdims=True)
    
    # Compute ground truth using brute force (for small dataset)
    print(f"Computing ground truth for {num_test} queries...")
    ground_truth = []
    for i, query in enumerate(test_vectors):
        # Compute L2 distances
        distances = np.sum((train_vectors - query) ** 2, axis=1)
        # Get k nearest neighbors
        indices = np.argsort(distances)[:k]
        ground_truth.append(indices)
    
    ground_truth = np.array(ground_truth)
    
    # Write files
    os.makedirs(output_dir, exist_ok=True)
    write_fvecs(os.path.join(output_dir, 'sift_base.fvecs'), train_vectors)
    write_fvecs(os.path.join(output_dir, 'sift_query.fvecs'), test_vectors)
    write_ivecs(os.path.join(output_dir, 'sift_groundtruth.ivecs'), ground_truth)
    
    print(f"Generated dataset:")
    print(f"  Train vectors: {num_train}")
    print(f"  Test vectors: {num_test}")
    print(f"  Dimension: {dim}")
    print(f"  Ground truth k: {k}")
    print(f"  Output: {output_dir}")

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    default_output = os.path.join(project_root, "data", "sift")
    output_dir = sys.argv[1] if len(sys.argv) > 1 else default_output
    
    # Generate small test dataset
    generate_sift_like_dataset(
        output_dir,
        num_train=10000,   # 10K vectors for quick testing
        num_test=100,      # 100 queries
        dim=128,
        k=100,
        seed=42
    )

if __name__ == "__main__":
    main()
