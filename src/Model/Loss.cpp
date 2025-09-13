#include "CNum/Model/Loss.h"

using namespace CNum::DataStructs;

namespace CNum::Model {
  // ******************
  // -----------------
  // Loss functions
  // -----------------
  // ******************
  //
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
    auto *act = Activation::get_activation_obj();

    double sum{0.0};

    for (int i = 0; i < y.get_rows(); i++) {
      double y_true = y.get(i, 0);
      auto sigmoid_output = act->activate(y_pred.get(i, 0), "sigmoid");
      double y_predicted = ::std::max(::std::min(sigmoid_output, 1 - BCE_CLIP_VAL), BCE_CLIP_VAL);

      sum += y_true * ::std::log(y_predicted) + (1 - y_true) * ::std::log(1 - y_predicted);
    }

    return -sum / y.get_rows();
  }

  double binary_crossentropy_gradient(double y,
				      double y_pred) {
    auto *act = Activation::get_activation_obj();
    return act->activate(y_pred, "sigmoid") - y;
  }

  double binary_crossentropy_hessian(double y,
				     double y_pred) {
    auto *act = Activation::get_activation_obj();
    double sigmoid_output = act->activate(y_pred, "sigmoid");
    return sigmoid_output * (1 - sigmoid_output);
  }

  // *****************
  // ----------------
  // Loss Singleton
  // ----------------
  // *****************

  Loss::Loss() {
    _loss_func_map["MSE"] = MSE_loss;
    _loss_func_gradient_map["MSE"] = MSE_gradient;
    _loss_func_hessian_map["MSE"] = MSE_hessian;

    _loss_func_map["RMSE"] = RMSE_loss;
    _loss_func_gradient_map["RMSE"] = MSE_gradient;
    _loss_func_hessian_map["RMSE"] = MSE_hessian;

    _loss_func_map["BCE"] = binary_crossentropy_loss;
    _loss_func_gradient_map["BCE"] = binary_crossentropy_gradient;
    _loss_func_hessian_map["BCE"] = binary_crossentropy_hessian;
  }

  Loss *Loss::get_loss_obj() {
    static Loss *loss = new Loss(); // leaks by design to avoid shutdown order issues
    return loss;
  }
 
  void Loss::get_gradients_hessians(const Matrix<double> &y,
				    const Matrix<double> &y_pred,
				    arena_view_t &g_out,
				    arena_view_t &h_out,
				    const arena_view_t &position_array,
				    ::std::string loss) {
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
      g_out_ptr[i] = _loss_func_gradient_map[loss](y.get(indeces[i], 0), y_pred.get(indeces[i], 0));
      h_out_ptr[i] = _loss_func_hessian_map[loss](y.get(indeces[i], 0), y_pred.get(indeces[i], 0));
    }
  }

  double Loss::get_loss(const Matrix<double> &y, const Matrix<double> &y_pred, ::std::string lf) {
    if (y.get_rows() != y_pred.get_rows()) {
      throw ::std::invalid_argument("Loss error - Misaligned dims");
    }

    if (y.get_cols() > 1 || y_pred.get_cols() > 1) {
      throw ::std::invalid_argument("Loss error - Only 1 dimensional matrices supported");
    }
  
    return _loss_func_map[lf](::std::cref(y), ::std::cref(y_pred));
  }
}
