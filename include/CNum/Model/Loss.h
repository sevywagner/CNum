#ifndef LOSS_H
#define LOSS_H

#include "CNum/Model/Activation.h"

#include <string>
#include <functional>
#include <cmath>

/**
 * @namespace CNum::Model::Loss
 * @brief Loss functions
 */
namespace CNum::Model::Loss {
  using GHFunction = std::function< double(double, double) >;
  using LossFunction = std::function< double(const CNum::DataStructs::Matrix<double> &,
					     const CNum::DataStructs::Matrix<double> &) >;
  

  /// @struct LossProfile
  /// @brief The loss, gradient, and hessian functions associated with a loss function
  struct LossProfile {
    LossFunction loss_func;
    GHFunction gradient_func;
    GHFunction hessian_func;
  };
  
  /// @brief Mean squared error
  /// @param y List of true y values (shape=(n,1))
  /// @param y_pred List of predicted values (shape=(n,1))
  /// @return The loss
  double MSE_loss(const CNum::DataStructs::Matrix<double> &y,
		  const CNum::DataStructs::Matrix<double> &y_pred);

  /// @brief Calculate the gradient value of the mean squared error
  /// @param y A true y sample
  /// @param y_pred A predicted y sample
  /// @return The gradient value
  double MSE_gradient(double y,
		      double y_pred);

  /// @brief Calculate the hessian (second order derivative) value of the mean
  /// squared error
  /// @param y A true y sample
  /// @param y_pred A predicted y sample
  /// @return The hessian value
  double MSE_hessian(double y,
		     double y_pred);

  /// @brief Root mean squared error
  /// @param y List of true y values (shape=(n,1))
  /// @param y_pred List of predicted values (shape=(n,1))
  /// @return The loss
  double RMSE_loss(const CNum::DataStructs::Matrix<double> &y,
		   const CNum::DataStructs::Matrix<double> &y_pred);

  /// @brief Binary crossentropy (log loss)
  /// @param y List of true y values (shape=(n,1))
  /// @param y_pred List of predicted values (shape=(n,1))
  /// @return The loss
  double binary_crossentropy_loss(const CNum::DataStructs::Matrix<double> &y,
				  const CNum::DataStructs::Matrix<double> &y_pred);

  /// @brief Calculate the gradient of the log loss
  /// @param y A true y sample
  /// @param y_pred A predicted y sample
  /// @return The gradient value
  double binary_crossentropy_gradient(double y,
				      double y_pred);

  /// @brief Calculate the hessian (second order derivative) of the log loss
   /// @param y A true y sample
  /// @param y_pred A predicted y sample
  /// @return The hessian value
  double binary_crossentropy_hessian(double y,
				     double y_pred);
  
  /// @brief Get the Gradients and Hessians of a Matrix
  /// @param y List of true y values (shape=(n,1)) 
  /// @param y_pred List of predicted values (shape=(n,1))
  /// @param g_out The arena_view_t to output the gradient values to
  /// @param h_out The arena_view_t to output the hessian values to
  /// @param position_array The arena_view_t containing the partitioned indeces
  /// of the data (partitions are for keeping track of which samples tree nodes
  /// have to work with)
  /// @param grad_func The GHFunction for the gradient of the loss
  /// @param hess_func The GHFunction for the hessian of the loss
  void get_gradients_hessians(const CNum::DataStructs::Matrix<double> &y,
			      const CNum::DataStructs::Matrix<double> &y_pred,
			      arena_view_t &g_out,
			      arena_view_t &h_out,
			      const arena_view_t &position_array,
			      GHFunction &grad_func,
			      GHFunction &hess_func);

  /// @brief Get the loss of a matrix of values
  /// @param y List of true y values (shape=(n,1)) 
  /// @param y_pred List of predicted values (shape=(n,1))
  /// @param loss_func The LossFunction to use
  /// @return The loss
  double get_loss(const CNum::DataStructs::Matrix<double> &y,
		  const CNum::DataStructs::Matrix<double> &y_pred,
		  LossFunction &loss_func);

  /// @brief Get the LossProfile associated with a string (i.e. "MSE" -> mean squared error function)
  /// @param loss The name of the loss function
  /// @return The LossProfile
  LossProfile get_loss_profile(::std::string loss);
  
};

#endif
