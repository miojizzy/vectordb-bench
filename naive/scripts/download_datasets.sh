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
PROJECT_ROOT="${SCRIPT_DIR}/.."
DATA_DIR="${PROJECT_ROOT}/data"
CONVERT_SCRIPT="${SCRIPT_DIR}/convert_hdf5_to_fvecs.py"

# URLs - All datasets from ann-benchmarks (HDF5 format)
SIFT_URL="http://ann-benchmarks.com/sift-128-euclidean.hdf5"
GLOVE_URL="http://ann-benchmarks.com/glove-50-angular.hdf5"
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

# Convert HDF5 to fvecs/ivecs format
convert_hdf5_dataset() {
  local hdf5_file=$1
  local output_dir=$2
  local use_dot_naming=$3

  log_info "Converting $hdf5_file to fvecs format..."

  if [ "$use_dot_naming" = "true" ]; then
    python3 "$CONVERT_SCRIPT" "$hdf5_file" "$output_dir" --dot
  else
    python3 "$CONVERT_SCRIPT" "$hdf5_file" "$output_dir"
  fi

  if [ $? -eq 0 ]; then
    rm -f "$hdf5_file"
    log_info "Converted and removed HDF5 file"
  else
    log_error "Conversion failed"
    return 1
  fi
}

# Download SIFT
download_sift() {
  log_step "Downloading SIFT dataset (L2 distance)..."

  local sift_dir="${DATA_DIR}/sift"
  local hdf5_file="${DATA_DIR}/sift-128-euclidean.hdf5"

  if [ -d "$sift_dir" ] && [ -f "$sift_dir/sift_base.fvecs" ]; then
    log_info "SIFT dataset already exists"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$SIFT_URL" "$hdf5_file"

  mkdir -p "$sift_dir"
  convert_hdf5_dataset "$hdf5_file" "$sift_dir" "false"
  log_info "SIFT dataset ready at $sift_dir"
}

# Download GloVe
download_glove() {
  log_step "Downloading GloVe dataset (Angular distance)..."

  local glove_dir="${DATA_DIR}/glove"
  local hdf5_file="${DATA_DIR}/glove-50-angular.hdf5"

  if [ -d "$glove_dir" ] && [ -f "$glove_dir/train.fvecs" ]; then
    log_info "GloVe dataset already exists"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$GLOVE_URL" "$hdf5_file"

  mkdir -p "$glove_dir"
  convert_hdf5_dataset "$hdf5_file" "$glove_dir" "true"
  log_info "GloVe dataset ready at $glove_dir"
}

# Download GIST
download_gist() {
  log_step "Downloading GIST dataset (L2 distance, 3.6GB)..."

  local gist_dir="${DATA_DIR}/gist"
  local hdf5_file="${DATA_DIR}/gist-960-euclidean.hdf5"

  if [ -d "$gist_dir" ] && [ -f "$gist_dir/train.fvecs" ]; then
    log_info "GIST dataset already exists"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$GIST_URL" "$hdf5_file"

  mkdir -p "$gist_dir"
  convert_hdf5_dataset "$hdf5_file" "$gist_dir" "true"
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
    log_error "GloVe: train.fvecs not found"
    all_ok=false
  fi

  # Verify GIST
  local gist_dir="${DATA_DIR}/gist"
  if [ -f "$gist_dir/train.fvecs" ]; then
    local size=$(stat -c%s "$gist_dir/train.fvecs" 2>/dev/null || stat -f%z "$gist_dir/train.fvecs")
    log_info "GIST: train.fvecs ($(($size / 1024 / 1024)) MB)"
  else
    log_error "GIST: train.fvecs not found"
    all_ok=false
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
  echo "Usage: $0 [sift|glove|gist|all|verify]"
  echo ""
  echo "Commands:"
  echo "  sift   - Download SIFT dataset (L2 distance, 501MB)"
  echo "  glove  - Download GloVe dataset (Angular distance, 235MB)"
  echo "  gist   - Download GIST dataset (L2 distance, 3.6GB)"
  echo "  all    - Download all datasets (default)"
  echo "  verify - Verify existing datasets"
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
    gist)
      download_gist
      ;;
    all)
      download_sift
      download_glove
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
