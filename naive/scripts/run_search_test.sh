#!/bin/bash

# HNSW 搜索性能测试脚本

set -e

BUILD_DIR="build"
EXECUTABLE="search_test"
DATA_DIR="${1:-data/sift-128-euclidean}"
MODES="${2:-baseline,DoubleM,HeuristicOnly,HeuristicPlusClosest}"
OUTPUT_FILE="${3:-/tmp/search_test_results.txt}"
LOCK_FILE="/tmp/search_test.lock"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}HNSW Search Performance Test${NC}"
echo -e "${GREEN}========================================${NC}"

# 检查是否已有测试进程在运行
if [ -f "$LOCK_FILE" ]; then
    PID=$(cat "$LOCK_FILE")
    if ps -p "$PID" > /dev/null 2>&1; then
        echo -e "${RED}Error: Test process (PID $PID) is already running${NC}"
        echo -e "${RED}If this is incorrect, remove $LOCK_FILE manually${NC}"
        exit 1
    else
        echo -e "${YELLOW}Warning: Stale lock file found. Removing...${NC}"
        rm -f "$LOCK_FILE"
    fi
fi

# 创建锁文件
echo $$ > "$LOCK_FILE"
trap "rm -f $LOCK_FILE" EXIT

# 检查构建目录
cd "$(dirname "$0")/.."

if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# 编译
cd "$BUILD_DIR"

if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${YELLOW}Executable not found. Compiling...${NC}"
    cmake ..
    make "$EXECUTABLE" -j$(nproc)
fi

# 检查数据集
if [ ! -d "../$DATA_DIR" ]; then
    echo -e "${RED}Error: Dataset not found: $DATA_DIR${NC}"
    echo -e "${YELLOW}Available datasets:${NC}"
    ls -d ../data/* 2>/dev/null || echo "  No datasets found"
    exit 1
fi

# 运行测试
echo -e "${GREEN}Starting search performance test...${NC}"
echo -e "  Dataset: ${YELLOW}$DATA_DIR${NC}"
echo -e "  Modes: ${YELLOW}$MODES${NC}"
echo -e "  Output: ${YELLOW}$OUTPUT_FILE${NC}"
echo ""

./$EXECUTABLE "../$DATA_DIR" "$MODES" "$OUTPUT_FILE"

if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}========================================${NC}"
    echo -e "${GREEN}Test completed successfully!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo -e "Results saved to: ${YELLOW}$OUTPUT_FILE${NC}"
    echo ""
    echo "View results:"
    echo -e "  ${YELLOW}cat $OUTPUT_FILE${NC}"
else
    echo -e "\n${RED}Test failed!${NC}"
    exit 1
fi
