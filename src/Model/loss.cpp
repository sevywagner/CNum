#include "CNum/Model/Loss.h"

using namespace CNum::DataStructs;

namespace CNum::Model::Loss {
  // -------------
  // MSE
  // -------------
  
  double MSE_loss(const Matrix<double> &y,
		  const Matrix<double> &y_pred) {
    double loss{ 0.0 };
    auto a = y - y_pred;
    a = a.squared();

    return a.sum() / y.get_rows();
  }

  double MSE_gradient(double y,
		      double y_pred) {
    return y_pred - y;
  }

  
  double MSE_hessian(double y,
		     double y_pred) {
    return 1.0;
  }

  // -------------
  // RMSE
  // -------------

  
  double RMSE_loss(const Matrix<double> &y,
		   const Matrix<double> &y_pred) {
    double loss = MSE_loss(y, y_pred);
  
    return sqrt(loss);
  }

  // --------------------
  // Binary Crossentropy
  // --------------------

  
  double binary_crossentropy_loss(const Matrix<double> &y,
				  const Matrix<double> &y_pred) {
    constexpr double BCE_CLIP_VAL = 1e-4;

    double sum{0.0};

    for (int i = 0; i < y.get_rows(); i++) {
      double y_true = y.get(i, 0);
      auto sigmoid_output = ::CNum::Model::Activation::sigmoid(y_pred.get(i, 0));
      double y_predicted = ::std::max(::std::min(sigmoid_output, 1 - BCE_CLIP_VAL), BCE_CLIP_VAL);

      sum += y_true * ::std::log(y_predicted) + (1 - y_true) * ::std::log(1 - y_predicted);
    }

    return -sum / y.get_rows();
  }

  double binary_crossentropy_gradient(double y,
				      double y_pred) {
    return ::CNum::Model::Activation::sigmoid(y_pred) - y;
  }

  double binary_crossentropy_hessian(double y,
				     double y_pred) {
    double sigmoid_output = ::CNum::Model::Activation::sigmoid(y_pred);
    return sigmoid_output * (1 - sigmoid_output);
  }
 
  void get_gradients_hessians(const Matrix<double> &y,
				    const Matrix<double> &y_pred,
				    arena_view_t &g_out,
				    arena_view_t &h_out,
				    const arena_view_t &position_array,
				    GHFunction &grad_func,
				    GHFunction &hess_func) {
    if (y.get_rows() != y_pred.get_rows()) {
      throw ::std::invalid_argument("GH error - Misaligned dims");
    }

    if (y.get_cols() > 1 || y_pred.get_cols() > 1) {
      throw ::std::invalid_argument("GH error - Only 1 dimensional matrices supported");
    }
    
    double *g_out_ptr = (double *) g_out.ptr;
    double *h_out_ptr = (double *) h_out.ptr;
    size_t *indeces = (size_t *) position_array.ptr;

    for (size_t i = 0; i < g_out.range; i++) {
      g_out_ptr[i] = grad_func(y.get(indeces[i], 0), y_pred.get(indeces[i], 0));
      h_out_ptr[i] = hess_func(y.get(indeces[i], 0), y_pred.get(indeces[i], 0));
    }
  }

  double get_loss(const Matrix<double> &y,
			const Matrix<double> &y_pred,
			LossFunction &loss_func) {
    if (y.get_rows() != y_pred.get_rows()) {
      throw ::std::invalid_argument("Loss error - Misaligned dims");
    }

    if (y.get_cols() > 1 || y_pred.get_cols() > 1) {
      throw ::std::invalid_argument("Loss error - Only 1 dimensional matrices supported");
    }
  
    return loss_func(::std::cref(y), ::std::cref(y_pred));
  }

  LossProfile get_loss_profile(::std::string loss) {
    if (loss == "MSE")
      return { MSE_loss, MSE_gradient, MSE_hessian };
    else if (loss == "RMSE")
      return { RMSE_loss, MSE_gradient, MSE_hessian };
    else if (loss == "BCE")
      return { binary_crossentropy_loss, binary_crossentropy_gradient, binary_crossentropy_hessian };
    else
      throw ::std::invalid_argument("Get loss profile error - Loss function \"" + loss + "\" not found");
  }
}
