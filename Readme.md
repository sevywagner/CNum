# CNum: A CPU-Optimized C++ ML Library

> **Status:** Alpha. APIs and results may change. 

## Highlights
- High-level ML interfaces with low-level control
- Easy to use data structures (Matrix, Mask, Hazard Pointer, Arena, Concurrent Queue, and more)
- Highly optimized Gradient Boosting Models
- Cache-friendly custom arena allocator and per-worker memory reuse
- Parallelized features using futures-based thread pool
- Global random number generator service (backed by xoshiro256)
- Optional REST API tools for inference (Crow based)

## Immediate Plans
- Implement LightGBM
- Implement autograd and tensor engine

## Dependencies
- <a href="https://github.com/nlohmann/json">nlohmann::json</a> - header-only, bundled
- <a href="https://github.com/ipkn/crow">Crow</a> - header-only, bundled, used only for the Deploy namespace (has asio as a dependency)
- <a href="https://think-async.com/Asio">asio</a> - only necessary if DEPLOY_TOOLS=ON
- <a href="https://github.com/google/googletest">GoogleTest</a> - Needed if BUILD_TESTS=ON (install with INSTALL_GTEST=ON)

## Supported Platforms
- Linux (GCC/Clang)
- MacOS (Clang)
- Windows (MSVC + CMake)

## Docs
Docs for this library can be found <a href="https://sevywagner.github.io/CNum">here</a>

## Quick Start
### Basic installation: 
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
sudo make install
```
All build examples assume CMAKE_BUILD_TYPE=Release, which is currently the only build type supported by CNum

### Using REST API tools:
To use REST API tools (optional) set DEPLOY_TOOLS=ON (OFF by default). While crow is bundled, it depends on asio (standalone or boost) which can be installed <a href="https://think-async.com/Asio">here</a>.
```bash
cmake -DDEPLOY_TOOLS=ON ..
```
In the event that CMake cannot locate the asio.hpp, file you must give it the path to the dir that contains asio.hpp by setting ASIO_HEADER_DIR.
```bash
cmake -DDEPLOY_TOOLS=ON -DASIO_HEADER_DIR=path/to/asio/include ..
```

To then use the deploy tools in your project, link the CNum_Deploy interface in your project's CMakeLists.txt:
```cmake
target_link_libraries(your_app PRIVATE CNum::CNum_Deploy)
```

Then include the Deploy interface in any source files using it:
```cpp
#include <CNum/Deploy.h>
```

The Deploy module is intentionally not included from CNum.h to keep it fully optional and to avoid imposing unnecessary Asio/Crow dependencies on users who do not need REST functionality.

### Building tests:
To build the test harness for CNum simply set BUILD_TESTS=ON (OFF by default).
```bash
cmake -DBUILD_TESTS=ON ..
```

If you do not have GoogleTest installed on your system you can set INSTALL_GTEST=ON (OFF by default) to install it
```bash
cmake -DBUILD_TESTS=ON -DINSTALL_GTEST=ON ..
```

Then to run the actual tests:
```bash
make
./tests/test_harness
```

## Build a test model
This example trains a Gradient Boosting regressor on a dummy dataset.

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

## License
CNum is released under the MIT license
