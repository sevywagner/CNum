# Changelog

## [0.2.1] - 2025-11-24
Testing and random number generation improvements

### Added:
- Started using Google Test for testing instead of DocTest
- Expanded the test suite with more meaningful tests
- Created a global service for generating random numbers
- Thread-local random generators with the ability to dynamically set the seed and reset the state
- Switched the random number generator from the ::std::mt19937 (Mersenne Twister) to the xoshiro256

### Known limitations:
- API subject to change
- Internal/advanced components still appear alongside public API components in the namespace hierarchy

[0.2.1]: https://github.com/sevywagner/CNum/releases/tag/v0.2.1

## [0.2.0] - 2025-11-21
Major cleanup and restructuring of internal systems

### Added:
- The ModelPool class for making pools of trained model instances
- The InferenceAPI class for creating REST APIs for inference
- Replaced the poorly designed Mask<MT, PT> class with BinaryMask and IndexMask
- Cross-Platform CI
- Replaced the Loss and Activation singletons with namespaces that have no mutable global state.
- Documentation available at https://sevywagner.github.io/CNum

### Known limitations
- API subject to change
- Test suite needs to be expanded
- Internal/advanced components appear alongside public API components in the namespace hierarchy

[0.2.0]: https://github.com/sevywagner/CNum/releases/tag/v0.2.0

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
