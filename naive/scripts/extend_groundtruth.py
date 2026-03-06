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

"""Extend groundtruth.ivecs using multiprocessing with fork.

Architecture:
  - Main process loads train/test data into memory
  - Fork worker pool (cpu-1 workers) with copy-on-write shared data
  - Main process dispatches query indices, collects results, writes to file
  - Workers compute top-k using NumPy (BLAS + argpartition)

Memory Model (Linux fork):
  - train/test data: shared via copy-on-write, no extra copy per worker
  - Each worker: O(n) for distance array + O(k) for result

Usage:
    python extend_groundtruth.py <data_dir> <new_k> [--distance <metric>]
    
Example:
    python extend_groundtruth.py data/sift-128-euclidean-topk100 1000
    python extend_groundtruth.py data/glove-50-angular-topk100 500 --distance angular
"""

import sys

# Check required dependencies before importing anything else
_missing = []
try:
  import numpy as np
except ImportError:
  _missing.append('numpy')
try:
  from tqdm import tqdm
except ImportError:
  _missing.append('tqdm')

if _missing:
  print(f'Error: Missing required packages: {", ".join(_missing)}')
  print('Install with: pip install ' + ' '.join(_missing))
  sys.exit(1)

import struct
import os
import argparse
import time
from multiprocessing import Pool, cpu_count


# ============================================================================
# File I/O Functions
# ============================================================================

def read_fvecs(filename: str) -> np.ndarray:
  """Read vectors from fvecs format."""
  vectors = []
  with open(filename, 'rb') as f:
    while True:
      dim_bytes = f.read(4)
      if not dim_bytes:
        break
      dim = struct.unpack('i', dim_bytes)[0]
      vec = np.frombuffer(f.read(dim * 4), dtype=np.float32)
      vectors.append(vec)
  return np.ascontiguousarray(vectors, dtype=np.float32)


def read_ivecs(filename: str) -> np.ndarray:
  """Read vectors from ivecs format."""
  vectors = []
  with open(filename, 'rb') as f:
    while True:
      dim_bytes = f.read(4)
      if not dim_bytes:
        break
      dim = struct.unpack('i', dim_bytes)[0]
      vec = np.frombuffer(f.read(dim * 4), dtype=np.int32)
      vectors.append(vec)
  return np.array(vectors, dtype=np.int32)


def write_ivecs(filename: str, vectors: np.ndarray) -> None:
  """Write vectors to ivecs format."""
  with open(filename, 'wb') as f:
    for vec in vectors:
      dim = len(vec)
      f.write(struct.pack('i', dim))
      f.write(vec.astype(np.int32).tobytes())


# ============================================================================
# Utility Functions
# ============================================================================

def format_time(seconds: float) -> str:
  if seconds < 0:
    return '--:--:--'
  if seconds < 60:
    return f'{seconds:.0f}s'
  elif seconds < 3600:
    mins, secs = divmod(int(seconds), 60)
    return f'{mins}m {secs}s'
  else:
    hours, remainder = divmod(int(seconds), 3600)
    mins, secs = divmod(remainder, 60)
    return f'{hours}h {mins}m {secs}s'


def format_number(n: int) -> str:
  return f'{n:,}'


def format_bytes(n: int) -> str:
  """Format bytes to human readable string."""
  for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
    if n < 1024:
      return f'{n:.1f} {unit}'
    n /= 1024
  return f'{n:.1f} PB'


# ============================================================================
# Worker Process Functions
# ============================================================================

# Global variables for worker processes (set via initializer)
_train_data = None
_test_data = None
_k = None
_distance_metric = None


def init_worker(train_data: np.ndarray, test_data: np.ndarray, 
                k: int, distance_metric: str):
  """Initialize worker process with shared data references.
  
  In Linux fork mode, train_data and test_data are shared via copy-on-write.
  Setting writeable=False prevents accidental modification that would trigger
  memory duplication.
  """
  global _train_data, _test_data, _k, _distance_metric
  _train_data = train_data
  _test_data = test_data
  _k = k
  _distance_metric = distance_metric
  
  # Protect shared data from accidental writes
  _train_data.flags.writeable = False
  _test_data.flags.writeable = False


def compute_topk(query_idx: int) -> tuple:
  """Compute top-k nearest neighbors for a single query.
  
  Args:
    query_idx: Index of the query in test_data
    
  Returns:
    Tuple of (query_idx, topk_indices) where topk_indices is sorted by distance
  """
  query = _test_data[query_idx]
  n_train = _train_data.shape[0]
  
  # Compute distances based on metric
  if _distance_metric in ['euclidean', 'l2']:
    # L2 distance: ||train - query||_2
    # Use BLAS-optimized norm computation
    dists = np.linalg.norm(_train_data - query, axis=1)
  elif _distance_metric in ['angular', 'cosine']:
    # Angular distance: 1 - cos(query, train)
    # Normalize both and compute dot product
    q_norm = query / (np.linalg.norm(query) + 1e-10)
    train_norms = _train_data / (np.linalg.norm(_train_data, axis=1, keepdims=True) + 1e-10)
    dists = 1 - np.dot(train_norms, q_norm)
  else:
    # Inner product: negate for minimization (higher = more similar)
    dists = -np.dot(_train_data, query)
  
  # Select top-k using argpartition (O(n) average)
  # argpartition gives unsorted top-k, need to sort them
  if _k >= n_train:
    topk_indices = np.argsort(dists)
  else:
    partition_indices = np.argpartition(dists, _k)[:_k]
    # Sort the top-k by distance
    topk_indices = partition_indices[np.argsort(dists[partition_indices])]
  
  return query_idx, topk_indices.astype(np.int32)


# ============================================================================
# Main Logic
# ============================================================================

def extend_groundtruth(data_dir: str, new_k: int, distance_metric: str = 'euclidean',
                       force: bool = False, num_workers: int = None) -> None:
  """Extend groundtruth with multiprocessing brute-force search.
  
  Architecture:
    1. Main process loads train/test data
    2. Fork worker pool with copy-on-write shared data
    3. Dispatch query indices to workers
    4. Collect results and write to file
  """
  # File paths
  base_path = os.path.join(data_dir, 'base.fvecs')
  query_path = os.path.join(data_dir, 'query.fvecs')
  gt_path = os.path.join(data_dir, 'groundtruth.ivecs')
  gt_backup_path = os.path.join(data_dir, 'groundtruth.ivecs.bak')
  gt_new_path = os.path.join(data_dir, f'groundtruth_{new_k}.ivecs')
  lock_path = os.path.join(data_dir, '.extend_groundtruth.lock')
  
  # Validate input files
  if not os.path.exists(base_path):
    print(f'Error: {base_path} not found')
    sys.exit(1)
  if not os.path.exists(query_path):
    print(f'Error: {query_path} not found')
    sys.exit(1)
  
  # Check if target file already exists
  if not force and os.path.exists(gt_new_path):
    try:
      existing_gt = read_ivecs(gt_new_path)
      if len(existing_gt.shape) == 2 and existing_gt.shape[1] >= new_k:
        print(f'Target file already exists: {gt_new_path}')
        print(f'  Existing k={existing_gt.shape[1]}, target k={new_k}')
        print(f'  Use --force to recompute')
        return
    except Exception as e:
      print(f'Warning: Failed to read existing file: {e}')
  
  # Check for lock file
  if os.path.exists(lock_path):
    print(f'Error: Lock file exists: {lock_path}')
    print('  Another process may be running. If not, delete the lock file.')
    sys.exit(1)
  
  # Create lock file
  with open(lock_path, 'w') as f:
    f.write(f'{os.getpid()}\n{time.strftime("%Y-%m-%d %H:%M:%S")}\n')
  
  try:
    # Load data in main process
    print(f'Loading base vectors from {base_path}...')
    train_data = read_fvecs(base_path)
    n_train, dim = train_data.shape
    print(f'  Shape: ({format_number(n_train)}, {dim})')
    print(f'  Memory: {format_bytes(train_data.nbytes)}')
    
    print(f'Loading query vectors from {query_path}...')
    test_data = read_fvecs(query_path)
    n_test = test_data.shape[0]
    print(f'  Shape: ({format_number(n_test)}, {dim})')
    print(f'  Memory: {format_bytes(test_data.nbytes)}')
    
    # Load existing groundtruth
    existing_k = 0
    if os.path.exists(gt_path):
      print(f'Loading existing groundtruth from {gt_path}...')
      existing_gt = read_ivecs(gt_path)
      existing_k = existing_gt.shape[1]
      print(f'  Existing k: {existing_k}')
    
    # Validate parameters
    if new_k > n_train:
      print(f'Warning: new_k ({new_k}) > base vectors ({n_train})')
      new_k = n_train
    
    if new_k <= existing_k:
      print(f'Error: new_k ({new_k}) <= existing k ({existing_k})')
      return
    
    # Determine number of workers
    if num_workers is None:
      num_workers = max(1, cpu_count() - 1)
    num_workers = min(num_workers, n_test)  # Don't need more workers than queries
    
    # Calculate chunksize for efficient communication
    chunksize = max(1, n_test // (num_workers * 10))
    
    print(f'\nConfiguration:')
    print(f'  Distance metric: {distance_metric}')
    print(f'  Target k: {new_k}')
    print(f'  Base vectors: {format_number(n_train)}')
    print(f'  Query vectors: {format_number(n_test)}')
    print(f'  Worker processes: {num_workers}')
    print(f'  Chunk size: {chunksize}')
    print(f'  NumPy version: {np.__version__}')
    print()
    
    # Protect data from accidental writes in main process too
    train_data.flags.writeable = False
    test_data.flags.writeable = False
    
    # Create result array
    results = np.empty((n_test, new_k), dtype=np.int32)
    
    # Create worker pool and process
    print('Computing nearest neighbors...')
    start_time = time.time()
    
    with Pool(processes=num_workers,
              initializer=init_worker,
              initargs=(train_data, test_data, new_k, distance_metric)) as pool:
      
      # Use imap_unordered for better throughput
      # Results arrive in arbitrary order, so we need query_idx to place correctly
      with tqdm(total=n_test, desc='  Searching', unit='query',
                ncols=100, bar_format='{l_bar}{bar}| {n_fmt}/{total_fmt} [{elapsed}<{eta}]') as pbar:
        
        for query_idx, topk_indices in pool.imap_unordered(
            compute_topk, range(n_test), chunksize=chunksize):
          results[query_idx] = topk_indices
          pbar.update(1)
    
    # Done
    total_time = time.time() - start_time
    total_ops = n_test * n_train
    print(f'\nComputation completed in {format_time(total_time)}')
    print(f'Throughput: {format_number(int(total_ops / total_time))} ops/s')
    print(f'New groundtruth shape: ({format_number(n_test)}, {new_k})')
    
    # Save results
    if os.path.exists(gt_path) and not os.path.exists(gt_backup_path):
      print(f'\nBacking up original to {gt_backup_path}...')
      os.rename(gt_path, gt_backup_path)
    
    print(f'Saving to {gt_new_path}...')
    write_ivecs(gt_new_path, results)
    
    print(f'Updating {gt_path}...')
    write_ivecs(gt_path, results)
    
    print('\nDone!')
    print(f'  Extended from k={existing_k} to k={new_k}')
    
  finally:
    if os.path.exists(lock_path):
      os.remove(lock_path)


# ============================================================================
# Entry Point
# ============================================================================

def main():
  parser = argparse.ArgumentParser(
    description='Extend groundtruth.ivecs with multiprocessing brute-force search',
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog='''
Examples:
  python extend_groundtruth.py data/sift-128-euclidean-topk100 1000
  python extend_groundtruth.py data/glove-50-angular-topk100 500 --distance angular
  python extend_groundtruth.py data/sift-128-euclidean-topk100 500 -w 4

Architecture:
  - Main process loads train/test data into memory
  - Fork worker pool (cpu-1 workers) with copy-on-write shared data
  - Workers compute top-k using NumPy BLAS + argpartition
  - Memory efficient: no data duplication per worker (Linux fork)
'''
  )
  parser.add_argument('data_dir', help='Directory containing base.fvecs, query.fvecs')
  parser.add_argument('new_k', type=int, help='New number of nearest neighbors')
  parser.add_argument('--distance', '-d', default='euclidean',
                      choices=['euclidean', 'l2', 'angular', 'cosine', 'ip', 'inner_product'],
                      help='Distance metric (default: euclidean)')
  parser.add_argument('--workers', '-w', type=int, default=None,
                      help='Number of worker processes (default: cpu-1)')
  parser.add_argument('--force', '-f', action='store_true',
                      help='Force recomputation')
  
  args = parser.parse_args()
  
  extend_groundtruth(args.data_dir, args.new_k, args.distance, args.force, args.workers)


if __name__ == '__main__':
  main()
