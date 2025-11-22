#include "CNum/Model/Activation.h"

using namespace CNum::DataStructs;

namespace CNum::Model::Activation {
  double sigmoid(double value) {
    return 1.0 / (1 + exp(-value));
  }

  Matrix<double> activate(const Matrix<double> &data, ActivationFunc act_func) noexcept {
    auto res = data;

    ::std::for_each(res.begin(), res.end(), [&act_func] (double &val) {
      val = act_func(val);
    });
      
    return res;
  }

  ActivationFunc get_activation_func(::std::string activation) {
    if (activation == "sigmoid")
      return sigmoid;
    else
      throw ::std::invalid_argument("Activation function \"" + activation + "\" not found.");
  }
};
