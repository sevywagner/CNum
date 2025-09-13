#ifndef __LINALG_H
#define __LINALG_H

#include "CNum/DataStructs/Matrix/Matrix.h"

namespace CNum::DataStructs::LinAlg {
  struct Eigen {
    ::std::unique_ptr<double[]> values;
    ::CNum::DataStructs::Matrix<double> vectors;
  };
  
  void unit_vector(::CNum::DataStructs::Matrix<double> &a);
  std::array< ::CNum::DataStructs::Matrix<double>, 2 > qr_decomposition(const ::CNum::DataStructs::Matrix<double> &a);
  Eigen find_eigen_values(const ::CNum::DataStructs::Matrix<double> &a);
  ::CNum::DataStructs::Matrix<double> covariance(const ::CNum::DataStructs::Matrix<double> &a);
};

#endif
