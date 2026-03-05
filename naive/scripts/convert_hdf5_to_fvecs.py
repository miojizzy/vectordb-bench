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

"""Convert HDF5 datasets from ann-benchmarks to fvecs/ivecs format."""

import h5py
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
  """Write vectors to ivecs format."""
  with open(filename, 'wb') as f:
    for vec in vectors:
      dim = len(vec)
      f.write(struct.pack('i', dim))
      f.write(vec.astype(np.int32).tobytes())


def convert_dataset(hdf5_path, output_dir):
  """Convert HDF5 dataset to fvecs/ivecs format.

  Args:
    hdf5_path: Path to input HDF5 file
    output_dir: Output directory for fvecs/ivecs files
  """
  os.makedirs(output_dir, exist_ok=True)

  print(f'Loading {hdf5_path}...')
  with h5py.File(hdf5_path, 'r') as f:
    print(f'  Keys: {list(f.keys())}')
    train = np.array(f['train'])
    test = np.array(f['test'])
    neighbors = np.array(f['neighbors'])

    # Convert to float32 if needed
    if train.dtype != np.float32:
      train = train.astype(np.float32)
    if test.dtype != np.float32:
      test = test.astype(np.float32)

    print(f'  Train: {train.shape}, dtype={train.dtype}')
    print(f'  Test: {test.shape}, dtype={test.dtype}')
    print(f'  Neighbors: {neighbors.shape}')

  # Use generic naming convention
  base_name = 'base.fvecs'
  query_name = 'query.fvecs'
  gt_name = 'groundtruth.ivecs'

  print(f'Writing {output_dir}/{base_name}...')
  write_fvecs(os.path.join(output_dir, base_name), train)
  print(f'Writing {output_dir}/{query_name}...')
  write_fvecs(os.path.join(output_dir, query_name), test)
  print(f'Writing {output_dir}/{gt_name}...')
  write_ivecs(os.path.join(output_dir, gt_name), neighbors)

  print('Conversion complete!')
  print(f'  Output directory: {output_dir}')
  print(f'  Files: {base_name}, {query_name}, {gt_name}')


def main():
  if len(sys.argv) < 2:
    print('Usage: python convert_hdf5_to_fvecs.py <hdf5_file> [output_dir]')
    sys.exit(1)

  hdf5_path = sys.argv[1]
  output_dir = sys.argv[2] if len(sys.argv) > 2 else os.path.dirname(hdf5_path)

  convert_dataset(hdf5_path, output_dir)


if __name__ == '__main__':
  main()
