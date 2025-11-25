# CNum: A CPU-Optimized C++ ML Library

> **Status:** Alpha. APIs and results may change. Test suite will be expanded in 0.3.0

## Highlights
- Makes writing ML code in C++ simple
- Easy to use data structures (Matrix, Mask, Hazard Pointer, Arena, Concurrent Queue, and more)
- Highly optimized Gradient Boosting Models
- Cache-friendly custom arena allocator and per-worker memory reuse
- Parallelized features using futures-based thread pool and C++ execution policies
- Designed for reproducible CPU performance
- Effortless REST APIs for inference
- No AI generated code used in the development of this library

## Immediate Plans
- More extensive tests
- Implement LightGBM
- Implement autograd and tensor engine

## Dependancies
The CNum library relies on the <a href="https://github.com/nlohmann/json">nlohmann json package</a> for the encoding of trained models to json format. As it is a header-only library the header is included in include/json.hpp. The CNum library also uses the <a href="https://github.com/ipkn/crow">Crow C++ microframework</a> for REST API tools used for the effortless inference API feature of CNum. The Crow library is also header only, however, it depends on the asio networking package (boost and standalone both work). If you intend to use the Deploy namespace of CNum for creating REST APIs for inference, you must first install the asio package which can be done <a href="https://think-async.com/Asio">here</a>. If you don't want to use the REST API tools, however this is not required (see "Quick Start").

## Docs
Docs for this library can be found <a href="https://sevywagner.github.io/CNum">here</a>

## Quick Start
When building CNum you can choose whether or not to include the REST API tools in the package. This can be done by simply setting the CMake DEPLOY_TOOLS option to "ON". By default this is turned off

### Build library
Default: 
```bash
mkdir build
cd build

# Use default settings (DEPLOY_TOOLS off)
cmake ..
sudo make install
```

With REST API tools:
```bash
mkdir build
cd build

# Use default settings (DEPLOY_TOOLS off)
cmake -DDEPLOY_TOOLS=ON ..
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
