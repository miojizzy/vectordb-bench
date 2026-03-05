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

"""
Synthetic dataset generator for vector search benchmarking.
Generates train/test vectors and groundtruth in fvecs/ivecs format.
"""

import argparse
import os
import struct
import numpy as np


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate synthetic dataset for vector search benchmarking"
    )
    parser.add_argument(
        "--train-size", type=int, default=50000,
        help="Number of training vectors (default: 50000)"
    )
    parser.add_argument(
        "--dim", type=int, default=128,
        help="Vector dimension (default: 128)"
    )
    parser.add_argument(
        "--test-size", type=int, default=5000,
        help="Number of test queries (default: 5000)"
    )
    parser.add_argument(
        "--topk", type=int, default=100,
        help="Ground truth top-k neighbors (default: 100)"
    )
    parser.add_argument(
        "--distance", type=str, default="euclidean",
        choices=["euclidean", "angular"],
        help="Distance metric (default: euclidean)"
    )
    parser.add_argument(
        "--output", type=str, default=None,
        help="Output directory (default: data/synthetic-{dim}-{distance}-topk{topk})"
    )
    parser.add_argument(
        "--seed", type=int, default=42,
        help="Random seed for reproducibility (default: 42)"
    )
    return parser.parse_args()


def generate_vectors(train_size, dim, test_size, top_k, distance, output_dir, seed):
    """Generate train/test vectors and groundtruth."""
    print(f"\nGenerating dataset with seed={seed}...")
    np.random.seed(seed)

    # Generate train vectors
    print(f"Generating {train_size} train vectors ({dim}-dim)...")
    if distance == "angular":
        # For angular distance, normalize vectors to unit sphere
        train_vectors = np.random.randn(train_size, dim).astype(np.float32)
        norms = np.linalg.norm(train_vectors, axis=1, keepdims=True)
        train_vectors = train_vectors / (norms + 1e-10)
    else:
        # Standard normal distribution for euclidean
        train_vectors = np.random.randn(train_size, dim).astype(np.float32)

    # Generate test vectors
    print(f"Generating {test_size} test queries...")
    if distance == "angular":
        test_vectors = np.random.randn(test_size, dim).astype(np.float32)
        norms = np.linalg.norm(test_vectors, axis=1, keepdims=True)
        test_vectors = test_vectors / (norms + 1e-10)
    else:
        test_vectors = np.random.randn(test_size, dim).astype(np.float32)

    # Write train vectors (base.fvecs)
    print(f"Writing {output_dir}/base.fvecs...")
    with open(f"{output_dir}/base.fvecs", 'wb') as f:
        for vec in train_vectors:
            f.write(struct.pack('i', dim))
            f.write(vec.tobytes())

    # Write test vectors (query.fvecs)
    print(f"Writing {output_dir}/query.fvecs...")
    with open(f"{output_dir}/query.fvecs", 'wb') as f:
        for vec in test_vectors:
            f.write(struct.pack('i', dim))
            f.write(vec.tobytes())

    # Compute ground truth (brute force)
    print(f"Computing ground truth (top-{top_k})...")
    if distance == "angular":
        # Angular distance = 1 - inner product (for normalized vectors)
        groundtruth = []
        for i, query in enumerate(test_vectors):
            inner_products = train_vectors @ query
            # For angular: closest = highest inner product
            nearest = np.argsort(-inner_products)[:top_k]
            groundtruth.append(nearest)
            if (i + 1) % 1000 == 0:
                print(f"  Processed {i+1}/{test_size} queries...")
    else:
        # Euclidean distance
        groundtruth = []
        for i, query in enumerate(test_vectors):
            diff = train_vectors - query
            dists = np.sum(diff * diff, axis=1)
            nearest = np.argsort(dists)[:top_k]
            groundtruth.append(nearest)
            if (i + 1) % 1000 == 0:
                print(f"  Processed {i+1}/{test_size} queries...")

    # Write ground truth (groundtruth.ivecs)
    print(f"Writing {output_dir}/groundtruth.ivecs...")
    with open(f"{output_dir}/groundtruth.ivecs", 'wb') as f:
        for gt in groundtruth:
            f.write(struct.pack('i', top_k))
            f.write(gt.astype(np.int32).tobytes())

    return train_size, dim, test_size, top_k, distance, output_dir


def print_stats(train_size, dim, test_size, top_k, distance, output_dir):
    """Print dataset statistics."""
    base_size_mb = os.path.getsize(f"{output_dir}/base.fvecs") / 1024 / 1024
    query_size_mb = os.path.getsize(f"{output_dir}/query.fvecs") / 1024 / 1024
    gt_size_mb = os.path.getsize(f"{output_dir}/groundtruth.ivecs") / 1024 / 1024

    print(f"\n==========================================")
    print(f"Dataset generated successfully!")
    print(f"==========================================")
    print(f"  Train vectors: {train_size}")
    print(f"  Test queries:  {test_size}")
    print(f"  Dimension:     {dim}")
    print(f"  Top-k:         {top_k}")
    print(f"  Distance:      {distance}")
    print(f"")
    print(f"File sizes:")
    print(f"  base.fvecs:         {base_size_mb:.2f} MB")
    print(f"  query.fvecs:        {query_size_mb:.2f} MB")
    print(f"  groundtruth.ivecs:  {gt_size_mb:.2f} MB")
    print(f"  Total:              {base_size_mb + query_size_mb + gt_size_mb:.2f} MB")
    print(f"==========================================")
    print(f"\nTo test with search_test:")
    print(f"  ./search_test {output_dir} baseline")


def main():
    args = parse_args()

    # Set default output directory if not specified
    if args.output is None:
        args.output = f"data/synthetic-{args.dim}-{args.distance}-topk{args.topk}"

    # Create output directory
    os.makedirs(args.output, exist_ok=True)

    print("==========================================")
    print("Synthetic Dataset Generator")
    print("==========================================")
    print(f"  Train size:  {args.train_size}")
    print(f"  Dimension:   {args.dim}")
    print(f"  Test size:   {args.test_size}")
    print(f"  Top-k:       {args.topk}")
    print(f"  Distance:    {args.distance}")
    print(f"  Output:      {args.output}")
    print(f"  Seed:        {args.seed}")
    print("==========================================")

    # Generate dataset
    result = generate_vectors(
        args.train_size, args.dim, args.test_size, args.topk,
        args.distance, args.output, args.seed
    )

    # Print statistics
    print_stats(*result)

    print("\nDone!")


if __name__ == "__main__":
    main()
