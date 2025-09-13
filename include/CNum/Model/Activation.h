#ifndef __ACTIVATION_H
#define __ACTIVATION_H

#include "CNum/DataStructs/DataStructs.h"

#include <functional>
#include <unordered_map>
#include <string>
#include <cmath>

namespace CNum::Model {
  using ActivationFunc = std::function< double(double) >;
  using ActivationMap = std::unordered_map< std::string, ActivationFunc >;
  
  double sigmoid(double value);

  class Activation {
  private:
    ActivationMap _activation_func_map;

    Activation();

  public:
    static Activation *get_activation_obj();

    Activation(const Activation &other) = delete;
    Activation(Activation &&other) = delete;

    Activation &operator=(const Activation &other) = delete;
    Activation &operator=(Activation &&other) = delete;
  
    ~Activation() = default;
  
    ::CNum::DataStructs::Matrix<double> activate(const ::CNum::DataStructs::Matrix<double> &data,
						 std::string activation) noexcept;

    double activate(double val,
		    std::string activation) noexcept;
  };
};

#endif
