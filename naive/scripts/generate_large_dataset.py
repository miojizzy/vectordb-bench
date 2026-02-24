#!/usr/bin/env python3
"""Generate larger test dataset for benchmark validation."""

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

    print('Generating training vectors...')
    train_vectors = np.random.randn(num_train, dim).astype(np.float32)
    train_vectors = train_vectors / np.linalg.norm(train_vectors, axis=1, keepdims=True)

    print('Generating test vectors...')
    test_vectors = np.random.randn(num_test, dim).astype(np.float32)
    test_vectors = test_vectors / np.linalg.norm(test_vectors, axis=1, keepdims=True)

    print('Computing ground truth...')
    ground_truth = []
    batch_size = 100
    for i in range(0, num_test, batch_size):
        batch_end = min(i + batch_size, num_test)
        for query in test_vectors[i:batch_end]:
            distances = np.sum((train_vectors - query) ** 2, axis=1)
            indices = np.argsort(distances)[:k]
            ground_truth.append(indices)
        print(f'  Progress: {batch_end}/{num_test}')

    ground_truth = np.array(ground_truth)

    os.makedirs('data/sift', exist_ok=True)
    write_fvecs('data/sift/sift_base.fvecs', train_vectors)
    write_fvecs('data/sift/sift_query.fvecs', test_vectors)
    write_ivecs('data/sift/sift_groundtruth.ivecs', ground_truth)

    print(f'Dataset generated: {num_train} train, {num_test} test, dim={dim}')

if __name__ == "__main__":
    main()
