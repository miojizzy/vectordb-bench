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

# ============================================================================
# 底层配置 (Infrastructure Configuration)
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."
DATA_DIR="${PROJECT_ROOT}/data"
CONVERT_SCRIPT="${SCRIPT_DIR}/convert_hdf5_to_fvecs.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ============================================================================
# 业务配置 (Business Configuration)
# ============================================================================

# Dataset configuration: name:dimension:distance:topk:url
declare -A DATASETS=(
  ["sift"]="128:euclidean:100:http://ann-benchmarks.com/sift-128-euclidean.hdf5"
  ["glove"]="50:angular:100:http://ann-benchmarks.com/glove-50-angular.hdf5"
  ["gist"]="960:euclidean:100:http://ann-benchmarks.com/gist-960-euclidean.hdf5"
)

# ============================================================================
# 底层逻辑 (Infrastructure Functions)
# ============================================================================

# Logging functions
log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Download file with resume support
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

  log_info "Converting $hdf5_file to fvecs format..."

  python3 "$CONVERT_SCRIPT" "$hdf5_file" "$output_dir"

  if [ $? -eq 0 ]; then
    rm -f "$hdf5_file"
    log_info "Converted and removed HDF5 file"
  else
    log_error "Conversion failed"
    return 1
  fi
}

# Get file size in MB
get_file_size_mb() {
  local file=$1
  local size=$(stat -c%s "$file" 2>/dev/null || stat -f%z "$file")
  echo $(($size / 1024 / 1024))
}

# ============================================================================
# 业务逻辑 (Business Functions)
# ============================================================================

# Download dataset by name
download_dataset() {
  local name=$1
  local config=${DATASETS[$name]}

  local dimension=$(echo "$config" | cut -d':' -f1)
  local distance=$(echo "$config" | cut -d':' -f2)
  local topk=$(echo "$config" | cut -d':' -f3)
  local url=$(echo "$config" | cut -d':' -f4-)

  local dataset_dir="${DATA_DIR}/${name}-${dimension}-${distance}-topk${topk}"
  local hdf5_file="${DATA_DIR}/${name}-${dimension}-${distance}.hdf5"

  log_step "Downloading ${name^^} dataset (${distance} distance, topk=${topk})..."

  if [ -d "$dataset_dir" ] && [ -f "$dataset_dir/base.fvecs" ]; then
    log_info "${name^^} dataset already exists"
    return 0
  fi

  mkdir -p "$DATA_DIR"
  download_file "$url" "$hdf5_file"

  mkdir -p "$dataset_dir"
  convert_hdf5_dataset "$hdf5_file" "$dataset_dir"
  log_info "${name^^} dataset ready at $dataset_dir"
}

# Verify single dataset
verify_dataset() {
  local name=$1
  local config=${DATASETS[$name]}

  local dimension=$(echo "$config" | cut -d':' -f1)
  local distance=$(echo "$config" | cut -d':' -f2)
  local topk=$(echo "$config" | cut -d':' -f3)

  local dataset_dir="${DATA_DIR}/${name}-${dimension}-${distance}-topk${topk}"
  local fvecs_file="${dataset_dir}/base.fvecs"

  if [ -f "$fvecs_file" ]; then
    local size=$(get_file_size_mb "$fvecs_file")
    log_info "${name^^}: base.fvecs (${size} MB)"
    return 0
  else
    log_error "${name^^}: base.fvecs not found"
    return 1
  fi
}

# Verify all datasets
verify() {
  log_step "Verifying datasets..."

  local all_ok=true

  for dataset_name in "sift" "glove" "gist"; do
    if ! verify_dataset "$dataset_name"; then
      all_ok=false
    fi
  done

  if $all_ok; then
    log_info "All datasets verified successfully!"
    return 0
  else
    log_error "Some datasets are missing!"
    return 1
  fi
}

# Show usage information
usage() {
  echo "Usage: $0 [sift|glove|gist|all|verify]"
  echo ""
  echo "Commands:"
  echo "  sift   - Download SIFT dataset (128-dim, L2 distance, 501MB)"
  echo "  glove  - Download GloVe dataset (50-dim, Angular distance, 235MB)"
  echo "  gist   - Download GIST dataset (960-dim, L2 distance, 3.6GB)"
  echo "  all    - Download all datasets (default)"
  echo "  verify - Verify existing datasets"
}

# ============================================================================
# Main Entry Point
# ============================================================================

main() {
  local command=${1:-all}

  log_info "VectorDB Benchmark Dataset Downloader"
  log_info "======================================"

  mkdir -p "$DATA_DIR"

  case $command in
    sift)
      download_dataset "sift"
      ;;
    glove)
      download_dataset "glove"
      ;;
    gist)
      download_dataset "gist"
      ;;
    all)
      for dataset_name in "sift" "glove" "gist"; do
        download_dataset "$dataset_name"
      done
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
