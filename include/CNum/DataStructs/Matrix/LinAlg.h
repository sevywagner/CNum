#ifndef LINALG_H
#define LINALG_H

#include "CNum/DataStructs/Matrix/Matrix.h"

#include <algorithm>
#include <array>

/**
 * @namespace CNum::DataStructs::LinAlg
 * @brief Linear algebra tools
 */
namespace CNum::DataStructs::LinAlg {
  /**
   * @struct Eigen
   * @brief Stores eigen vectors and eigen values
   */
  struct Eigen {
    ::std::unique_ptr<double[]> values;
    ::CNum::DataStructs::Matrix<double> vectors;
  };

  /**
   * @struct QR
   * @brief Contains a Q matrix and an R matrix resulant of QR decomposition
   */
  struct QR {
    ::CNum::DataStructs::Matrix<double> q;
    ::CNum::DataStructs::Matrix<double> r;
  };

  /// @brief Calculate the Frobenius norm of a Matrix
  /// @param m The matrix
  /// @param is_off_diagonal Whether or not to only take the norm of the off-diagonal part
  /// @return The norm
  double frobenius_norm(const Matrix<double> &m, bool is_off_diagonal = false);

  /// @brief Get single column unit vector
  /// @param a The vector of which we want to find the unit vector (shape=(n, 1))
  /// @return The unit vector
  void unit_vector(::CNum::DataStructs::Matrix<double> &a);

  /// @brief QR Decomposition
  /// @param a The matrix to decompose
  /// @return The Q and R matrices
  QR qr_decomposition(const ::CNum::DataStructs::Matrix<double> &a);

  /// @brief Get Eigen Values and Eigen Vectors of matrix
  /// @param a The matrix we want to find the eigen vectors and values of
  /// @return An Eigen struct with the eigen vectors and values
  Eigen find_eigen_values(const ::CNum::DataStructs::Matrix<double> &a);

  /// @brief Get covariance matrix
  /// @param a Matrix to get the covariance matrix of
  /// @return The covariance matrix
  ::CNum::DataStructs::Matrix<double> covariance(const ::CNum::DataStructs::Matrix<double> &a);
};

#endif
