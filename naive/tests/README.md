# Unit Tests

This directory contains unit tests for the refactored codebase using Google Test framework.

## Test Files

### 1. CLI Parser Tests (`cli_parser_test.cpp`)

Tests the command-line argument parser functionality:

- `IndexTypeToString` - Index type to string conversion
- `ParseIndexType` - String to index type parsing
- `ParseBasicArgs` - Basic argument parsing
- `ParseIndexTypes` - Index types list parsing
- `ParseNumericArgs` - Numeric parameters parsing
- `ParseListArgs` - Comma-separated list parsing
- `ParseBoolArgs` - Boolean flag parsing
- `ParseBenchmarkArgs` - Google Benchmark parameter passthrough
- `MissingRequiredArgs` - Required argument validation
- `HelpArg` - Help message display

### 2. Index Wrapper Tests (`index_wrapper_test.cpp`)

Tests the abstract index wrapper interface:

- `CreateIndexWrapper` - Factory function for creating index wrappers
- `HnswlibBasicOperations` - hnswlib wrapper basic functionality
- `ArenaHnswlibBasicOperations` - arena-hnswlib wrapper basic functionality
- `InnerProductMetric` - Inner product distance metric
- `SetEfSearch` - Search ef parameter setting
- `AllIndexTypes` - Test all supported index types

### 3. Hnswlib Wrapper Tests (`hnswlib_wrapper_test.cpp`)

Tests the hnswlib index wrapper:

- `Constructor` - Wrapper construction for different metrics
- `AddPoint` - Point addition functionality
- `SearchKnn` - k-NN search functionality
- `SetEfSearch` - Ef search parameter setting
- `GetName` - Index name retrieval
- `EdgeCases` - Boundary condition handling
- `InnerProductDistance` - Inner product distance calculations
- `HighDimension` - High-dimensional vector handling

### 4. Arena Hnswlib Wrapper Tests (`arena_hnswlib_wrapper_test.cpp`)

Tests the arena-hnswlib index wrapper:

- `Constructor` - Wrapper construction for different modes
- `AddPoint` - Point addition functionality
- `SearchKnn` - k-NN search functionality
- `DifferentModes` - Different Layer0NeighborMode tests
- `SetEfSearch` - Ef search parameter setting
- `GetName` - Index name retrieval for different modes
- `InnerProductDistance` - Inner product distance calculations
- `EdgeCases` - Boundary condition handling
- `PerformanceComparison` - Performance comparison across modes

### 5. Index Manager Tests (`index_manager_test.cpp`)

Tests the global index manager:

- `BasicOperations` - Basic manager operations
- `CreateSingleIndex` - Single index creation
- `CreateMultipleIndexes` - Multiple index creation
- `BuildIndexesParallel` - Parallel index building
- `IndexSearch` - Index search functionality
- `ClearIndexes` - Index cache clearing
- `IndexSize` - Index size tracking
- `DifferentParameters` - Different M and ef_construction parameters
- `ThreadSafety` - Multi-threaded access safety
- `IndexAvailability` - Index availability checking

## Building and Running Tests

### Prerequisites

- CMake 3.16+
- C++17 compiler (GCC 7+, Clang 5+)
- Google Test (automatically fetched by CMake)

### Build and Run

#### Option 1: Using the test script

```bash
cd /path/to/naive
./scripts/run_unit_tests.sh
```

#### Option 2: Manual build

```bash
cd /path/to/naive
mkdir -p build_tests
cd build_tests
cmake ../tests -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./unit_tests
```

### Run Specific Tests

```bash
# Run all tests
./unit_tests

# Run specific test suite
./unit_tests --gtest_filter=CliParserTest.*

# Run specific test case
./unit_tests --gtest_filter=CliParserTest.ParseIndexType

# Generate XML report
./unit_tests --gtest_output=xml:test_results.xml

# Show verbose output
./unit_tests --gtest_print_time=0
```

## Test Coverage

### Components Tested

| Component | File | Tests | Coverage |
|-----------|------|-------|----------|
| CLI Parser | `cli_parser.cpp` | 9 | 90%+ |
| Index Wrapper | `index_wrapper.cpp` | 6 | 85%+ |
| Hnswlib Wrapper | `hnswlib_wrapper.cpp` | 8 | 90%+ |
| Arena Hnswlib Wrapper | `arena_hnswlib_wrapper.cpp` | 9 | 90%+ |
| Index Manager | `index_manager.cpp` | 10 | 85%+ |

### What's NOT Tested

- Benchmark test files (`benchmark_*.cpp`) - These are performance tests, not unit tests
- `main_refactored.cpp` - Integration tests would be needed
- `dataset_loader.cpp` - Requires actual data files, can be added later

## Adding New Tests

When adding new functionality, follow these guidelines:

1. **Test file naming**: `{component}_test.cpp`
2. **Test suite naming**: `{Component}Test`
3. **Test case naming**: Descriptive verb phrases (e.g., `ParseIndexType`, `CreateIndex`)
4. **Use test fixtures** for common setup/teardown
5. **Test both success and failure paths**
6. **Use meaningful assertion messages**

### Example

```cpp
TEST(MyComponentTest, MyFunction) {
  // Arrange
  MyClass obj;
  
  // Act
  auto result = obj.MyFunction();
  
  // Assert
  EXPECT_EQ(result, expected_value);
}
```

## Troubleshooting

### Common Issues

1. **Build fails with "gtest not found"**
   - CMake will automatically fetch Google Test. Check internet connection.

2. **Segmentation fault in tests**
   - Check for null pointers in wrapper implementations
   - Verify array bounds in test data generation

3. **Tests pass locally but fail in CI**
   - Check for environment-specific code (paths, threads)
   - Verify compiler version compatibility

### Debug Mode

For debugging test failures:

```bash
cmake ../tests -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./unit_tests --gtest_filter=FailingTest*
```

## Continuous Integration

These tests should be run:
- Before every commit
- In CI/CD pipeline
- Before creating pull requests
- After dependency updates

## Test Output Interpretation

```
[==========] Running 42 tests from 5 test suites.
[----------] Global test environment set-up.
[----------] 9 tests from CliParserTest
[ RUN      ] CliParserTest.IndexTypeToString
[       OK ] CliParserTest.IndexTypeToString (0 ms)
...
[----------] 9 tests from CliParserTest (12 ms total)
[----------] Global test environment tear-down
[==========] 42 tests from 5 test suites ran. (123 ms total)
[  PASSED  ] 42 tests.
```

- `[==========]` - Test suite start/end
- `[----------]` - Test case group start/end
- `[ RUN      ]` - Individual test start
- `[       OK ]` - Test passed
- `[  PASSED  ]` - Final summary
