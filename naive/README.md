# HNSWLIB Benchmark (Naive)

A minimal benchmark for comparing hnswlib and arena-hnswlib performance using Google Benchmark.

## Features

- Support for L2 (Euclidean) and Inner Product distance metrics
- Multiple datasets: SIFT, GloVe, Fashion-MNIST, Last.fm
- Google Benchmark integration
- Recall calculation
- **Comparison between hnswlib and arena-hnswlib**

## Libraries

| Library | GitHub | Description |
|---------|--------|-------------|
| hnswlib | https://github.com/nmslib/hnswlib | Header-only HNSW implementation |
| arena-hnswlib | https://github.com/miojizzy/arena-hnswlib | Optimized HNSW with aligned memory |

## Prerequisites

- C++17 compiler (GCC 9+ or Clang 10+)
- CMake 3.16+
- Python 3.7+ (for dataset generation)

## Build

```bash
cd naive

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Datasets

### Download Real Datasets

```bash
cd naive

# Fashion-MNIST (L2 distance, 60K train, 10K test, 784 dim)
wget http://ann-benchmarks.com/fashion-mnist-784-euclidean.hdf5 -O data/fashion-mnist.hdf5
python3 scripts/convert_hdf5_to_fvecs.py data/fashion-mnist.hdf5 data/fashion-mnist

# Last.fm (Dot product, 292K train, 50K test, 65 dim)
wget http://ann-benchmarks.com/lastfm-64-dot.hdf5 -O data/lastfm-dot.hdf5
python3 scripts/convert_hdf5_to_fvecs.py data/lastfm-dot.hdf5 data/lastfm --dot
```

### Generate Synthetic Dataset

```bash
cd naive
python3 scripts/generate_large_dataset.py   # SIFT-style (L2)
python3 scripts/generate_ip_dataset.py      # GloVe-style (IP)
```

## Run

```bash
# Test Fashion-MNIST (L2 distance)
./build/hnswlib_bench data/fashion-mnist

# Test Last.fm (Dot product)
./build/hnswlib_bench data/lastfm

# Test synthetic SIFT dataset (L2 distance)
./build/hnswlib_bench data/sift

# Test synthetic GloVe dataset (Inner Product distance)
./build/hnswlib_bench data/glove
```

## Benchmark Results

### Fashion-MNIST (L2, 60K train, 10K test, 784 dim)

#### Index Construction

| Library | M | ef_construction | Time | QPS |
|---------|---|-----------------|------|-----|
| hnswlib | 16 | 200 | 22.1s | 2.7K/s |
| arena-hnswlib | 16 | 200 | 24.7s | 2.4K/s |
| hnswlib | 32 | 200 | 23.4s | 2.6K/s |
| arena-hnswlib | 32 | 200 | 22.4s | 2.7K/s |

#### Search Performance

| Library | ef_search | QPS | Recall@100 |
|---------|-----------|-----|------------|
| hnswlib | 50 | 4.0K/s | **99.3%** |
| arena-hnswlib | 50 | 3.8K/s | 97.3% |
| hnswlib | 200 | 2.2K/s | **99.9%** |
| arena-hnswlib | 200 | 2.5K/s | 99.5% |
| hnswlib | 500 | 1.2K/s | **100.0%** |
| arena-hnswlib | 500 | 1.3K/s | 99.9% |

### Last.fm (Dot Product, 292K train, 50K test, 65 dim)

#### Index Construction

| Library | M | ef_construction | Time | QPS |
|---------|---|-----------------|------|-----|
| hnswlib | 16 | 200 | 47.3s | 6.2K/s |
| arena-hnswlib | 16 | 200 | 64.9s | 4.5K/s |
| hnswlib | 32 | 200 | 49.1s | 6.0K/s |
| arena-hnswlib | 32 | 200 | 75.9s | 3.9K/s |

#### Search Performance

| Library | ef_search | QPS | Recall@100 |
|---------|-----------|-----|------------|
| hnswlib | 50 | 13.7K/s | **96.0%** |
| arena-hnswlib | 50 | 9.5K/s | 90.7% |
| hnswlib | 200 | 8.7K/s | **98.8%** |
| arena-hnswlib | 200 | 7.0K/s | 97.1% |
| hnswlib | 500 | 4.2K/s | **99.8%** |
| arena-hnswlib | 500 | 3.1K/s | 99.4% |

## Key Findings

### L2 Distance (Fashion-MNIST)
1. **Index Construction**: Both libraries similar, hnswlib slightly faster
2. **Search Quality**: hnswlib achieves higher recall at same ef_search
3. **Recommendation**: Use **hnswlib** for L2 distance

### Inner Product Distance (Last.fm)
1. **Index Construction**: hnswlib is **~37% faster**
2. **Search Quality**: hnswlib achieves **~5% higher recall**
3. **Recommendation**: Use **hnswlib** for Inner Product distance

### Why arena-hnswlib Underperforms?
- arena-hnswlib's InnerProduct space lacks SIMD optimization (pure loop)
- L2 space has AVX optimization, but hnswlib still wins in most cases

## Notes

- Results use real benchmark datasets from ann-benchmarks.com
- Both libraries achieve excellent recall (>95%) with proper ef_search tuning
- hnswlib is the better choice for both L2 and Inner Product metrics
