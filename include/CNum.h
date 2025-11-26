#ifndef CNUM_H
#define CNUM_H

#include "CNum/DataStructs/DataStructs.h"
#include "CNum/Data/Data.h"
#include "CNum/Utils/Utils.h"
#include "CNum/Multithreading/Multithreading.h"
#include "CNum/Model/Model.h"

/** \mainpage Intro to CNum
 * \section About
 * CNum is a CPU-optimized machine learning library for C++ that provides
 * user-friendly ML model interfaces for training and inference, while exposing 
 * more low-level features that allow for control over thread 
 * management and memory allocation. CNum currently offers a 
 * Gradient Boosting model abstraction which can be trained with any type of tree booster to 
 * solve regression and classification problems. Currently the only type of tree booster 
 * the CNum library offers is an XGBoost-style tree booster. CNum also offers an 
 * extension for creating REST APIs for inference. 
 * The design principles behind the creation of CNum were maintaining numerical stability and model 
 * accuracy  while optimizing for efficiency and 
 * cache-friendliness. These principles are demonstrated in CNum's use of deterministic
 * random number generators and algorithms such as Kahan's summation
 * algorithm which aid in numerical stability and reproducibility. They are also demonstrated in the
 * use of row-wise features, and thread-local arena allocators for cache-friendliness. CNum is
 * currently still in the early stages of development, but in the future will also contain deep
 * learning tools like an autograd and tensor engine, neurons, and layers.
 */

/**
 * @namespace CNum
 * @brief The umbrella namespace from which you can access all modules of CNum
 */
namespace CNum {};

#endif
