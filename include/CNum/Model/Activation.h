#ifndef ACTIVATION_H
#define ACTIVATION_H

#include "CNum/DataStructs/DataStructs.h"

#include <functional>
#include <unordered_map>
#include <string>
#include <cmath>

/**
 * @namespace CNum::Model::Activation
 * @brief Activation functions
 */
namespace CNum::Model::Activation {
  using ActivationFunc = std::function< double(double) >;

  /// @brief Sigmoid function (for a single value)
  /// @param value The x value in the sigmoid function
  /// @return The result of the sigmoid function
  double sigmoid(double value);

  /// @brief Run an activation function on a Matrix of data
  /// @param data The data to run the activation function on
  /// @param act_func The activation function
  /// @return The matrix of values resulting from the activation function
  ::CNum::DataStructs::Matrix<double> activate(const ::CNum::DataStructs::Matrix<double> &data,
					       ActivationFunc act_func) noexcept;

  /// @brief Get an activation function from a string
  /// @param activation The name of the activation function (i.e. "sigmoid")
  /// @return The ActivationFunc
  ActivationFunc get_activation_func(::std::string activation);
};

#endif
