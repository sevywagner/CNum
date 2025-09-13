#include "CNum/DataStructs/Matrix/LinAlg.h"

namespace CNum::DataStructs::LinAlg {
  // ---- Get single column unit vector ----
  void unit_vector(Matrix<double> &a) {
    if (a.get_cols() > 1) {
      throw ::std::invalid_argument("Unit vector error - Only supports 1 column");
    }
  
    double magnitude = std::transform_reduce(std::execution::par_unseq,
					     a.begin(),
					     a.end(),
					     0.0,
					     std::plus<double>(),
					     [] (double &val) {
					       return val * val;
					     });
    magnitude = sqrt(magnitude);

    std::for_each(std::execution::par_unseq, a.begin(), a.end(), [magnitude] (double &val) {
      val /= magnitude;
    });
  }

  // ---- QR (Strang) Decomposition numerical method ----
  std::array< ::CNum::DataStructs::Matrix<double>, 2 > qr_decomposition(const Matrix<double> &a) {
    std::vector< Matrix<double> > q;
    std::array< Matrix<double>, 2 > qr;
    auto r_ptr = std::make_unique<double[]>(a.get_rows() * a.get_cols());

    for (int i = 0; i < a.get_cols(); i++) {
      auto v = a.get(COL, i);
      auto a_i = v;
    
      for (int j = i - 1; j >= 0; j--) {
	v = v + (q[j] * -1 * (q[j].dot(v)));
	r_ptr[j * a.get_cols() + i] = q[j].dot(a_i);
      }
    
      unit_vector(v);
      q.push_back(v);

      r_ptr[i * a.get_cols() + i] = a_i.dot(q[i]);
    }

    qr[0] = Matrix<double>::join_cols(q);
    qr[1] = Matrix<double>(a.get_rows(), a.get_cols(), std::move(r_ptr));
  
    return qr;
  }

  // ---- Get Eigen Values and Eigen Vectors of matrix ----
  Eigen find_eigen_values(const Matrix<double> &a) {
    auto qr = qr_decomposition(a);
    auto eigen_vectors = Matrix<double>::identity(qr[0].get_rows());
    auto b = qr[1] * qr[0];

    size_t last_row = a.get_rows() - 1;
    
    // *Update to check for convergence*
    for (int i = 0; i < 50; i++) {
      qr = qr_decomposition(b);
      b = qr[1] * qr[0];
      eigen_vectors = eigen_vectors * qr[0];
    }

    size_t len = std::min(b.get_rows(), b.get_cols());
    auto eigen_values = std::make_unique<double[]>(len);

    for (int i = 0; i < len; i++) {
      eigen_values[i] = b.get(i, i);
    }
    return { ::std::move(eigen_values), eigen_vectors };
  }

  // ---- Get covariance matrix ----
  Matrix<double> covariance(const Matrix<double> &a) {
    std::vector< Matrix<double> > cols;
  
    for (int i = 0; i < a.get_cols(); i++) {
      auto col = a.get(COL, i);
      double mean = col.mean();
      col = col + -mean;
      cols.push_back(col);
    }
  
    auto x = Matrix<double>::join_cols(cols);
    return (x.transpose() * x) * (1.0 / (a.get_cols() - 1));
  }
};
