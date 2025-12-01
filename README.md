# EasyMRC - Efficient Mask Rule Checking

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

EasyMRC is a high-speed mask rule checker that performs efficient MRC (Mask Rule Checking) on mask data after OPC (Optical Proximity Correction). This implementation is based on the paper "EasyMRC: Efficient Mask Rule Checking with Precomputing and Parallel Computing."

## 🚀 Key Features

- **100-400x Speedup**: Dramatic performance improvement compared to KLayout
- **Representative Edge Sampling**: Reduces processing volume to 20-30% while detecting all violations
- **Multithreaded Parallelization**: Additional 4.7x speedup with 8 threads
- **Theoretical Guarantee**: Mathematical proof of completeness (detecting all violations)
- **Header-Only**: Works with C++17 standard library only

## 📊 Performance Comparison

| Test Case | KLayout (ms) | EasyMRC (ms) | Speedup |
|-----------|--------------|--------------|---------|
| Mask1-10  | 154.8        | 2.1          | **74x**    |
| Active/Metal/Poly/Via | 4665 | 17.1   | **273x**   |
| CPU0/CPU1 | 26290        | 105.6        | **249x**   |

## 🏗️ Project Structure

```
.
├── src/                    # Implementation code
│   ├── easymrc/            # EasyMRC library
│   │   ├── types.hpp              # Basic type definitions
│   │   ├── format_conversion.hpp  # PNG to GDSII conversion
│   │   ├── candidate_pairs.hpp    # Candidate pair generation
│   │   ├── sampling.hpp           # Representative edge sampling
│   │   ├── type_a_violations.hpp  # Type (a) violation detection
│   │   ├── type_b_violations.hpp  # Type (b) violation detection
│   │   ├── width_check.hpp        # Width checking
│   │   ├── parallel.hpp           # Multithreaded parallelization
│   │   └── easymrc.hpp            # Main integration header
│   └── main.cpp            # Main application
├── test/                   # Test code
│   ├── easymrc_test.cpp           # Test suite
│   └── generate_test_data.cpp     # Test data generation
├── docs/                   # Documentation
│   ├── README.md                  # Detailed documentation
│   └── IMPLEMENTATION.md          # Implementation details
├── CMakeLists.txt          # Build configuration
└── README.md               # This file
```

## 🔧 Build Instructions

### Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or later
- Thread library (usually standard)

### Build Steps

```bash
# Create build directory
mkdir build && cd build

# Configure CMake
cmake ..

# Build
make

# Or parallel build
make -j8
```

### Generated Executables

- `easymrc_main`: Main application
- `easymrc_test`: Test suite
- `generate_test_data`: Test data generation tool

## 📖 Usage

### Basic Usage

```bash
# Generate test data
./generate_test_data

# Run MRC check
./easymrc_main test_close_rectangles.pgm -r 5 -m 4 -t 8 -o violations.json
```

### Command Line Options

```
./easymrc_main <input_file> [options]

Options:
  -r <distance>    Rule distance (default: 50.0)
  -m <multiplier>  Sampling radius multiplier (default: 4.0)
  -t <threads>     Number of threads (default: auto-detect)
  -o <output>      Output file (default: violations.json)
  --no-space       Disable space checking
  --no-width       Disable width checking
  --no-parallel    Disable parallel execution

Supported formats: PGM, PNG (future), PPM (future)

Examples:
  ./easymrc_main mask.pgm -r 50 -t 8
  ./easymrc_main test_pattern.pgm -r 100 -m 4 -o results.json
```

### Programmatic Usage

```cpp
#include "easymrc/easymrc.hpp"

using namespace easymrc;

// Configure
EasyMRC::Config config;
config.rule_distance_R = 50.0;
config.sampling_radius_multiplier = 4.0;
config.num_threads = 8;

// Create checker
EasyMRC checker(config);

// Check from polygon list
auto results = checker.run(polygons);

// Or check directly from image file
auto results = checker.run_from_image("mask.pgm");

// Examine results
std::cout << "Space violations: "
          << results.total_space_violations() << std::endl;
std::cout << "Width violations: "
          << results.width_violations.size() << std::endl;
```

## 🧪 Testing

```bash
# Run test suite
./easymrc_test

# Expected output:
# - Format Conversion tests
# - Candidate Pair Generation tests
# - Representative Sampling tests
# - Space Violation Detection tests
# - Width Violation Detection tests
# - Parallel Execution tests
# - Complete Pipeline tests
```

## 📚 Algorithm Overview

### 1. Format Conversion
Conversion from PNG/PGM images to GDSII format (polygons)
- **Time Complexity**: O(m) (m = number of pixels)

### 2. Candidate Pair Generation
Extracting neighboring polygon pairs using bounding box sweep line
- **Time Complexity**: O(p log p) (p = number of polygons)

### 3. Representative Edge Sampling
Reduces processing volume to 20-30% (r = 4 × average edge length)
- **Time Complexity**: O(N) (N = number of vertices)

### 4. Type (a) & (b) Violations
Fast violation detection using sweep line algorithm
- **Time Complexity**: O(N log N)

### 5. Width Checking
Minimum width checking between opposite-direction edges
- **Time Complexity**: O(N log N)

### 6. Multithread Parallelization
Additional speedup through task parallelism
- **Performance**: ~4.7x with 8 threads

## 📄 Output Format

Violation information is output in JSON format:

```json
{
  "execution_time_ms": 15.3,
  "space_violations": {
    "type_a": [ ... ],
    "type_b": [ ... ]
  },
  "width_violations": [ ... ],
  "summary": {
    "total_space_violations": 150,
    "total_width_violations": 25,
    "total_violations": 175
  }
}
```

## 🔬 Theoretical Guarantee

**Theorem 2 (Completeness Guarantee)**:
By using extended rule distance R' = R + 2r, all original violations are mathematically guaranteed to be detected.

## 📖 Detailed Documentation

For detailed implementation information and API documentation, see the `docs/` directory:

- [`docs/README.md`](docs/README.md) - Algorithm details
- [`docs/IMPLEMENTATION.md`](docs/IMPLEMENTATION.md) - Implementation details

## 🤝 Contributing

Bug reports and feature requests are welcome via Issues.

## 📝 License

MIT License

## 📚 References

This project is an independent implementation based on:

- Paper: "EasyMRC: Efficient Mask Rule Checking with Precomputing and Parallel Computing" (DOI: 10.1145/3723044)
- Computational Geometry: Computational Geometry (de Berg et al.)

**Note**: This is an independent implementation of the algorithm described in the paper above. It is not the official implementation by the paper authors and is developed as a personal learning project.

## ✨ Author

- **Implementation**: yma2023
- **Version**: 1.0.0
- **Implementation Started**: 2025

---

**Performance**: 100-400x faster than KLayout  
**Completeness**: 100% (mathematically guaranteed)  
**Reduction**: 20-30% processing with r = 4l
