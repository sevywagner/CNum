#include "CNum/Model/Activation.h"

using namespace CNum::DataStructs;

namespace CNum::Model {
  double sigmoid(double value) {
    return 1.0 / (1 + exp(-value));
  }

  // ---------------------
  // Activation Singleton
  // ---------------------

  Activation::Activation() {
    _activation_func_map["sigmoid"] = sigmoid;
  }

  Activation *Activation::get_activation_obj() {
    static Activation *activation = new Activation(); // leaks by design to avoid shutdown order issues
    return activation;
  }

  Matrix<double> Activation::activate(const Matrix<double> &data, std::string activation) noexcept {
    auto act_func = _activation_func_map[activation];

    auto res = data;

    ::std::for_each(::std::execution::par_unseq,
		    res.begin(),
		    res.end(),
		    [&, this] (double &val) {
      val = act_func(val);
    });
      
    return res;
  }

  double Activation::activate(double val,
			      std::string activation) noexcept {
    return _activation_func_map[activation](val);
  }
};

