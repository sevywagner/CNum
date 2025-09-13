#ifndef __LOSS_H
#define __LOSS_H

#include "CNum/Model/Activation.h"

#include <string>
#include <map>
#include <functional>
#include <cmath>

namespace CNum::Model {
  using GHFunction = std::function< double(double, double) >;
  using LossFunction = std::function< double(const CNum::DataStructs::Matrix<double> &,
					     const CNum::DataStructs::Matrix<double> &) >;
  using GHMap = std::unordered_map< std::string, GHFunction >;
  using LossMap = std::unordered_map< std::string, LossFunction >;
  
  
  double MSE_loss(const CNum::DataStructs::Matrix<double> &y,
		  const CNum::DataStructs::Matrix<double> &y_pred);
  
  double MSE_gradient(double y,
		      double y_pred);
  
  double MSE_hessian(double y,
		     double y_pred);
  
  double RMSE_loss(const CNum::DataStructs::Matrix<double> &y,
		   const CNum::DataStructs::Matrix<double> &y_pred);

  double binary_crossentropy_loss(const CNum::DataStructs::Matrix<double> &y,
				  const CNum::DataStructs::Matrix<double> &y_pred);
  
  double binary_crossentropy_gradient(double y,
				      double y_pred);
  
  double binary_crossentropy_hessian(double y,
				     double y_pred);

  class Loss {
  private:
    LossMap _loss_func_map;
    GHMap _loss_func_gradient_map;
    GHMap _loss_func_hessian_map;

    Loss();

  public:
    static Loss *get_loss_obj();

    Loss(const Loss &other) = delete;
    Loss(Loss &&other) = delete;

    Loss &operator=(const Loss &other) = delete;
    Loss &operator=(Loss &&other) = delete;
  
    ~Loss() = default;
  
    void get_gradients_hessians(const CNum::DataStructs::Matrix<double> &y,
				const CNum::DataStructs::Matrix<double> &y_pred,
				arena_view_t &g_out,
				arena_view_t &h_out,
				const arena_view_t &position_array,
				std::string loss);
    double get_loss(const CNum::DataStructs::Matrix<double> &y,
		    const CNum::DataStructs::Matrix<double> &y_pred,
		    std::string lf);
  
  };
};

#endif
