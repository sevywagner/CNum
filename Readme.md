# CNum: A CPU-Optimized C++ ML Library

> **Status:** Pre-alpha. APIs and results may change. Currently tested on Linux (Gentoo). Cross-platform CI and full test coverage are coming in upcoming releases.

## Highlights
- Makes writing ML code in C++ simple
- Easy to use data structures (Matrix, Mask, Hazard Pointer, Arena, Concurrent Queue, and more)
- Highly optimized Gradient Boosting Models
- Cache-friendly custom arena allocator and per-worker memory reuse
- Parallelized features using futures-based thread pool and C++ execution policies
- Designed for reproducible CPU performance
- No AI generated code used in the development of this library

## Immediate Plans
- Write and upload documentation and design notes
- Add cross-platform CI and more tests
- Implement LightGBM
- Implement model pools for inference APIs

## Quick Start
### Build library
```bash
mkdir build
cd build
cmake ..
sudo make install
```

### Build a test model
main.cpp
```cpp
#include <CNum.h>
#include <memory>
#include <numeric>
	 
using namespace CNum::DataStructs;
using namespace CNum::Model::Tree;

int main() {
    constexpr size_t len = 100;

    auto x = ::std::make_unique<double[]>(len);
    auto y = ::std::make_unique<double[]>(len);
    ::std::iota(x.get(), x.get() + len, 0.0);
    ::std::iota(y.get(), y.get() + len, 0.0);

    Matrix<double> x_matrix(len, 1, ::std::move(x));
    Matrix<double> y_matrix(len, 1, ::std::move(y));

    GBModel<XGTreeBooster> xgboost("MSE", 100 /* n_learners */);
    xgboost.fit(x_matrix, y_matrix);
    auto preds = xgboost.predict(x_matrix);
    preds.print_matrix();

    return 0;
}
```

CMakeLists.txt
```txt
cmake_minimum_required(VERSION 3.20)

project(test_proj VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CNum CONFIG REQUIRED)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE CNum::CNum)
```

```bash
mkdir build
cd build
cmake ..
make
./main
```