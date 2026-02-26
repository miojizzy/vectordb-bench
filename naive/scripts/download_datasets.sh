#!/bin/bash
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

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="${SCRIPT_DIR}/../data"

# URLs - Using multiple mirrors for reliability
SIFT_URL="ftp://ftp.irisa.fr/local/texmex/corpus/sift.tar.gz"
SIFT_MIRROR="http://ann-benchmarks.com/sift-128-euclidean.tar.gz"
GLOVE_URL="http://ann-benchmarks.com/glove-50-angular.hdf5"
DEEP1B_URL="http://ann-benchmarks.com/deep-96-angular.hdf5"
GIST_URL="http://ann-benchmarks.com/gist-960-euclidean.hdf5"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Download function with resume support
download_file() {
  local url=$1
  local output=$2

  if [ -f "$output" ]; then
    log_info "File already exists: $output"
    return 0
  fi

  log_info "Downloading $url..."

  if command -v wget &> /dev/null; then
    wget -c "$url" -O "$output"
  elif command -v curl &> /dev/null; then
    curl -C - -L "$url" -o "$output"
  else
    log_error "Neither wget nor curl found. Please install one of them."
    exit 1
  fi
}

# Download and extract SIFT
download_sift() {
  log_step "Downloading SIFT dataset (L2 distance)..."

  local sift_dir="${DATA_DIR}/sift"
  local tar_file="${DATA_DIR}/sift.tar.gz"

  if [ -d "$sift_dir" ] && [ -f "$sift_dir/sift_base.fvecs" ]; then
    log_info "SIFT dataset already exists"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$SIFT_URL" "$tar_file"

  log_info "Extracting SIFT dataset..."
  mkdir -p "$sift_dir"
  tar -xzf "$tar_file" -C "$sift_dir" --strip-components=1 2>/dev/null || \
    tar -xzf "$tar_file" -C "$sift_dir"

  # Rename files if needed
  if [ -f "$sift_dir/sift-128-euclidean.train" ]; then
    mv "$sift_dir/sift-128-euclidean.train" "$sift_dir/sift_base.fvecs" 2>/dev/null || true
    mv "$sift_dir/sift-128-euclidean.test" "$sift_dir/sift_query.fvecs" 2>/dev/null || true
    mv "$sift_dir/sift-128-euclidean.groundtruth" "$sift_dir/sift_groundtruth.ivecs" 2>/dev/null || true
  fi

  rm -f "$tar_file"
  log_info "SIFT dataset ready at $sift_dir"
}

# Download GloVe
download_glove() {
  log_step "Downloading GloVe dataset (Angular/Inner Product distance)..."

  local glove_dir="${DATA_DIR}/glove"
  local hdf5_file="${DATA_DIR}/glove-50-angular.hdf5"

  if [ -d "$glove_dir" ] && [ -f "$glove_dir/train.fvecs" ]; then
    log_info "GloVe dataset already extracted"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$GLOVE_URL" "$hdf5_file"

  log_info "Extracting GloVe dataset from HDF5..."
  mkdir -p "$glove_dir"

  # Use Python to extract HDF5 to fvecs format
  python3 << EOF
import h5py
import numpy as np
import struct
import os

hdf5_path = "${hdf5_file}"
output_dir = "${glove_dir}"

print(f"Loading {hdf5_path}...")
with h5py.File(hdf5_path, 'r') as f:
    # Print available keys
    print(f"Keys: {list(f.keys())}")

    # Load data
    train = np.array(f['train'])
    test = np.array(f['test'])
    neighbors = np.array(f['neighbors'])
    distances = np.array(f.get('distances', None))

    print(f"Train shape: {train.shape}")
    print(f"Test shape: {test.shape}")
    print(f"Neighbors shape: {neighbors.shape}")

# Write train vectors to fvecs
def write_fvecs(filename, vectors):
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.float32).tobytes())

# Write ground truth to ivecs
def write_ivecs(filename, vectors):
    with open(filename, 'wb') as f:
        for vec in vectors:
            dim = len(vec)
            f.write(struct.pack('i', dim))
            f.write(vec.astype(np.int32).tobytes())

write_fvecs(os.path.join(output_dir, 'train.fvecs'), train)
write_fvecs(os.path.join(output_dir, 'test.fvecs'), test)
write_ivecs(os.path.join(output_dir, 'groundtruth.ivecs'), neighbors)

print("Extraction complete!")
EOF

  rm -f "$hdf5_file"
  log_info "GloVe dataset ready at $glove_dir"
}

# Download DEEP1B
download_deep1b() {
  log_step "Downloading DEEP1B dataset (Angular distance, 3.6GB)..."

  local deep1b_dir="${DATA_DIR}/deep1b"
  local hdf5_file="${DATA_DIR}/deep-96-angular.hdf5"

  if [ -d "$deep1b_dir" ] && [ -f "$deep1b_dir/train.fvecs" ]; then
    log_info "DEEP1B dataset already extracted"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$DEEP1B_URL" "$hdf5_file"

  log_info "Extracting DEEP1B dataset from HDF5..."
  mkdir -p "$deep1b_dir"

  # Use Python to extract HDF5 to fvecs format
  python3 << 'EOF'
import h5py
import numpy as np
import struct
import os
import sys

hdf5_path = os.environ.get('HDF5_PATH', '')
output_dir = os.environ.get('OUTPUT_DIR', '')

print(f"Loading {hdf5_path}...")
try:
    with h5py.File(hdf5_path, 'r') as f:
        print(f"Keys: {list(f.keys())}")

        train = np.array(f['train'])
        test = np.array(f['test'])
        neighbors = np.array(f['neighbors'])

        print(f"Train shape: {train.shape}")
        print(f"Test shape: {test.shape}")
        print(f"Neighbors shape: {neighbors.shape}")

    def write_fvecs(filename, vectors):
        with open(filename, 'wb') as f:
            for i, vec in enumerate(vectors):
                if i % 1000000 == 0:
                    print(f"  Writing vector {i}/{len(vectors)}...")
                dim = len(vec)
                f.write(struct.pack('i', dim))
                f.write(vec.astype(np.float32).tobytes())

    def write_ivecs(filename, vectors):
        with open(filename, 'wb') as f:
            for vec in vectors:
                dim = len(vec)
                f.write(struct.pack('i', dim))
                f.write(vec.astype(np.int32).tobytes())

    write_fvecs(os.path.join(output_dir, 'train.fvecs'), train)
    write_fvecs(os.path.join(output_dir, 'test.fvecs'), test)
    write_ivecs(os.path.join(output_dir, 'groundtruth.ivecs'), neighbors)

    print("Extraction complete!")
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)
EOF

  # Export variables for the Python script
  export HDF5_PATH="$hdf5_file"
  export OUTPUT_DIR="$deep1b_dir"
  python3 << 'PYEOF'
import h5py
import numpy as np
import struct
import os
import sys

hdf5_path = os.environ['HDF5_PATH']
output_dir = os.environ['OUTPUT_DIR']

print(f"Loading {hdf5_path}...")
try:
    with h5py.File(hdf5_path, 'r') as f:
        print(f"Keys: {list(f.keys())}")

        train = np.array(f['train'])
        test = np.array(f['test'])
        neighbors = np.array(f['neighbors'])

        print(f"Train shape: {train.shape}")
        print(f"Test shape: {test.shape}")
        print(f"Neighbors shape: {neighbors.shape}")

    def write_fvecs(filename, vectors):
        with open(filename, 'wb') as f:
            for i, vec in enumerate(vectors):
                if i % 1000000 == 0:
                    print(f"  Writing vector {i}/{len(vectors)}...")
                dim = len(vec)
                f.write(struct.pack('i', dim))
                f.write(vec.astype(np.float32).tobytes())

    def write_ivecs(filename, vectors):
        with open(filename, 'wb') as f:
            for vec in vectors:
                dim = len(vec)
                f.write(struct.pack('i', dim))
                f.write(vec.astype(np.int32).tobytes())

    write_fvecs(os.path.join(output_dir, 'train.fvecs'), train)
    write_fvecs(os.path.join(output_dir, 'test.fvecs'), test)
    write_ivecs(os.path.join(output_dir, 'groundtruth.ivecs'), neighbors)

    print("Extraction complete!")
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)
PYEOF

  rm -f "$hdf5_file"
  log_info "DEEP1B dataset ready at $deep1b_dir"
}

# Download GIST
download_gist() {
  log_step "Downloading GIST dataset (Euclidean distance, 3.6GB)..."

  local gist_dir="${DATA_DIR}/gist"
  local hdf5_file="${DATA_DIR}/gist-960-euclidean.hdf5"

  if [ -d "$gist_dir" ] && [ -f "$gist_dir/train.fvecs" ]; then
    log_info "GIST dataset already extracted"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$GIST_URL" "$hdf5_file"

  log_info "Extracting GIST dataset from HDF5..."
  mkdir -p "$gist_dir"

  # Export variables for the Python script
  export HDF5_PATH="$hdf5_file"
  export OUTPUT_DIR="$gist_dir"
  python3 << 'PYEOF'
import h5py
import numpy as np
import struct
import os
import sys

hdf5_path = os.environ['HDF5_PATH']
output_dir = os.environ['OUTPUT_DIR']

print(f"Loading {hdf5_path}...")
try:
    with h5py.File(hdf5_path, 'r') as f:
        print(f"Keys: {list(f.keys())}")

        train = np.array(f['train'])
        test = np.array(f['test'])
        neighbors = np.array(f['neighbors'])

        print(f"Train shape: {train.shape}")
        print(f"Test shape: {test.shape}")
        print(f"Neighbors shape: {neighbors.shape}")

    def write_fvecs(filename, vectors):
        with open(filename, 'wb') as f:
            for i, vec in enumerate(vectors):
                if i % 100000 == 0:
                    print(f"  Writing vector {i}/{len(vectors)}...")
                dim = len(vec)
                f.write(struct.pack('i', dim))
                f.write(vec.astype(np.float32).tobytes())

    def write_ivecs(filename, vectors):
        with open(filename, 'wb') as f:
            for vec in vectors:
                dim = len(vec)
                f.write(struct.pack('i', dim))
                f.write(vec.astype(np.int32).tobytes())

    write_fvecs(os.path.join(output_dir, 'train.fvecs'), train)
    write_fvecs(os.path.join(output_dir, 'test.fvecs'), test)
    write_ivecs(os.path.join(output_dir, 'groundtruth.ivecs'), neighbors)

    print("Extraction complete!")
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)
PYEOF

  rm -f "$hdf5_file"
  log_info "GIST dataset ready at $gist_dir"
}

# Verify datasets
verify() {
  log_step "Verifying datasets..."

  local all_ok=true

  # Verify SIFT
  local sift_dir="${DATA_DIR}/sift"
  if [ -f "$sift_dir/sift_base.fvecs" ]; then
    local size=$(stat -c%s "$sift_dir/sift_base.fvecs" 2>/dev/null || stat -f%z "$sift_dir/sift_base.fvecs")
    log_info "SIFT: sift_base.fvecs ($(($size / 1024 / 1024)) MB)"
  else
    log_error "SIFT: sift_base.fvecs not found"
    all_ok=false
  fi

  # Verify GloVe
  local glove_dir="${DATA_DIR}/glove"
  if [ -f "$glove_dir/train.fvecs" ]; then
    local size=$(stat -c%s "$glove_dir/train.fvecs" 2>/dev/null || stat -f%z "$glove_dir/train.fvecs")
    log_info "GloVe: train.fvecs ($(($size / 1024 / 1024)) MB)"
  else
    log_warn "GloVe: train.fvecs not found (optional)"
  fi

  # Verify DEEP1B
  local deep1b_dir="${DATA_DIR}/deep1b"
  if [ -f "$deep1b_dir/train.fvecs" ]; then
    local size=$(stat -c%s "$deep1b_dir/train.fvecs" 2>/dev/null || stat -f%z "$deep1b_dir/train.fvecs")
    log_info "DEEP1B: train.fvecs ($(($size / 1024 / 1024)) MB)"
  else
    log_warn "DEEP1B: train.fvecs not found (optional)"
  fi

  # Verify GIST
  local gist_dir="${DATA_DIR}/gist"
  if [ -f "$gist_dir/train.fvecs" ]; then
    local size=$(stat -c%s "$gist_dir/train.fvecs" 2>/dev/null || stat -f%z "$gist_dir/train.fvecs")
    log_info "GIST: train.fvecs ($(($size / 1024 / 1024)) MB)"
  else
    log_warn "GIST: train.fvecs not found (optional)"
  fi

  if $all_ok; then
    log_info "All datasets verified successfully!"
    return 0
  else
    log_error "Some datasets are missing!"
    return 1
  fi
}

# Usage
usage() {
  echo "Usage: $0 [sift|glove|deep1b|gist|all|verify]"
  echo ""
  echo "Commands:"
  echo "  sift    - Download SIFT dataset (L2 distance, 501MB)"
  echo "  glove   - Download GloVe dataset (Angular distance, 235MB)"
  echo "  deep1b  - Download DEEP1B dataset (Angular distance, 3.6GB)"
  echo "  gist    - Download GIST dataset (L2 distance, 3.6GB)"
  echo "  all     - Download all datasets (default)"
  echo "  verify  - Verify existing datasets"
}

# Main
main() {
  local command=${1:-all}

  log_info "VectorDB Benchmark Dataset Downloader"
  log_info "======================================"

  mkdir -p "$DATA_DIR"

  case $command in
    sift)
      download_sift
      ;;
    glove)
      download_glove
      ;;
    deep1b)
      download_deep1b
      ;;
    gist)
      download_gist
      ;;
    all)
      download_sift
      download_glove
      download_deep1b
      download_gist
      ;;
    verify)
      verify
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      log_error "Unknown command: $command"
      usage
      exit 1
      ;;
  esac

  verify
}

main "$@"
