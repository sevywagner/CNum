# Changelog

## [0.1.0] — 2025-09-14
Initial pre-alpha release of CNum.

### Added:
- Easy-to-use gradient boosting models highly optimized for CPU
- Model save/load
- Core data structures such as matrices, masks, concurrent queues, hazard pointers, arenas, and views
- Linear algebra utilities
- Uniform bin and quantile sketch subroutines
- Thread Pool for parallel tasks
- CMake packaging

### Known limitations
- API subject to change
- Only tested on Gentoo Linux

[0.1.0]: https://github.com/sevywagner/CNum/releases/tag/v0.1.0

## [0.2.0] - 2025-11-21
A cleaner release of CNum

### Added:
- The ModelPool class for making pools of trained model instances
- The InferenceAPI class for creating REST APIs for inference effortlessly
- Replaced the poorly designed Mask<MT, PT> class with BinaryMask and IndexMask
- Cross-Platform CI
- Replaced the Loss and Activation singletons with namespaces that have no mutable global state.
- Documentation available <a href="https://sevywagner.github.io/CNum">here</a>

### Known limitations
- API subject to change
- Test suite needs to be expanded
- Internals/Advanced components appear at the same level in the class hierarchies

[0.2.0]: https://github.com/sevywagner/CNum/releases/tag/v0.2.0
