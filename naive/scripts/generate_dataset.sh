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

# ============================================================================
# Usage
# ============================================================================

usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --train-size N    Number of training vectors (default: 50000)"
    echo "  --dim N           Vector dimension (default: 128)"
    echo "  --test-size N     Number of test queries (default: 5000)"
    echo "  --topk N          Ground truth top-k neighbors (default: 100)"
    echo "  --distance TYPE   Distance metric: euclidean or angular (default: euclidean)"
    echo "  --output DIR      Output directory (default: data/synthetic-\${DIM}-\${DISTANCE})"
    echo "  --seed N          Random seed for reproducibility (default: 42)"
    echo ""
    echo "Examples:"
    echo "  $0                                    # Use defaults"
    echo "  $0 --train-size 10000 --dim 64        # Small dataset"
    echo "  $0 --train-size 100000 --dim 256     # Larger dataset"
    echo "  $0 --topk 50 --distance angular       # Custom top-k and metric"
}

# ============================================================================
# Main
# ============================================================================

if [[ "$#" -eq 0 ]] || [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
    usage
    exit 0
fi

# Run Python script with all arguments
exec python3 "${SCRIPT_DIR}/generate_dataset.py" "$@"
