#!/usr/bin/env python3
"""Generate Inner Product distance dataset for benchmark validation.

For Inner Product / Cosine similarity:
- Vectors are L2 normalized
- Ground truth is computed using Inner Product (negative IP for ranking)
"""

import numpy as np
import struct
import os

def write_fvecs(filename, vectors):
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.float32).tobytes())

def write_ivecs(filename, vectors):
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.int32).tobytes())

def main():
    np.random.seed(42)
    num_train = 50000  # 50K vectors
    num_test = 1000    # 1K queries
    dim = 128
    k = 100

    print('Generating training vectors (L2 normalized)...')
    train_vectors = np.random.randn(num_train, dim).astype(np.float32)
    # L2 normalize for Inner Product / Cosine similarity
    train_vectors = train_vectors / np.linalg.norm(train_vectors, axis=1, keepdims=True)

    print('Generating test vectors (L2 normalized)...')
    test_vectors = np.random.randn(num_test, dim).astype(np.float32)
    # L2 normalize for Inner Product / Cosine similarity
    test_vectors = test_vectors / np.linalg.norm(test_vectors, axis=1, keepdims=True)

    # Verify normalization
    norms = np.linalg.norm(train_vectors, axis=1)
    print(f'Train vector norms: min={norms.min():.6f}, max={norms.max():.6f}, mean={norms.mean():.6f}')

    print('Computing ground truth using Inner Product distance...')
    # For Inner Product: higher IP = more similar
    # We want to find vectors with highest IP (most similar)
    # Using negative IP for argsort (smallest negative = highest IP)
    ground_truth = []
    batch_size = 100
    for i in range(0, num_test, batch_size):
        batch_end = min(i + batch_size, num_test)
        for query in test_vectors[i:batch_end]:
            # Inner Product: sum of element-wise products
            # Higher IP = more similar, so we negate for argsort
            ip_scores = np.dot(train_vectors, query)
            # Get k highest IP (most similar)
            indices = np.argsort(-ip_scores)[:k]  # negate to get descending order
            ground_truth.append(indices)
        print(f'  Progress: {batch_end}/{num_test}')

    ground_truth = np.array(ground_truth)

    os.makedirs('data/glove', exist_ok=True)
    write_fvecs('data/glove/train.fvecs', train_vectors)
    write_fvecs('data/glove/test.fvecs', test_vectors)
    write_ivecs('data/glove/groundtruth.ivecs', ground_truth)

    print(f'\nDataset generated for Inner Product distance:')
    print(f'  Train vectors: {num_train}')
    print(f'  Test vectors: {num_test}')
    print(f'  Dimension: {dim}')
    print(f'  K: {k}')
    print(f'  Metric: Inner Product (vectors are L2 normalized)')
    print(f'  Output: data/glove/')

if __name__ == "__main__":
    main()
