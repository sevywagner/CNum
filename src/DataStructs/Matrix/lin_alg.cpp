#include "CNum/DataStructs/Matrix/LinAlg.h"

namespace CNum::DataStructs::LinAlg {
  void unit_vector(Matrix<double> &a) {
    if (a.get_cols() > 1) {
      throw ::std::invalid_argument("Unit vector error - Only supports 1 column");
    }
  
    double magnitude = std::transform_reduce(a.begin(),
  					     a.end(),
					     0.0,
					     std::plus<double>(),
					     [] (double &val) {
					       return val * val;
					     });
    magnitude = sqrt(magnitude);

    std::for_each(a.begin(), a.end(), [magnitude] (double &val) {
      val /= magnitude;
    });
  }

  QR qr_decomposition(const Matrix<double> &a) {
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
  
    return { Matrix<double>::join_cols(q), Matrix<double>(a.get_rows(), a.get_cols(), std::move(r_ptr)) };
  }

  
  double frobenius_norm(const Matrix<double> &m, bool is_off_diagonal) {
    double norm{ 0.0 };
    size_t cols = m.get_cols();
    size_t rows = m.get_rows();

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	if (is_off_diagonal && i == j)
	  continue;
	
	auto val = m.get(i, j);
	norm += val * val;
      }
    }

    return ::std::sqrt(norm);
  }

  Eigen find_eigen_values(const Matrix<double> &a) {
    constexpr double convergence_tol = 1e-10;
    constexpr int max_iter = 1000;
    
    auto qr = qr_decomposition(a);
    auto eigen_vectors = Matrix<double>::identity(qr.q.get_rows());
    auto b = qr.r * qr.q;

    size_t last_row = a.get_rows() - 1;
    int iter{ 0 };
    
    while (iter++ < max_iter) {
      qr = qr_decomposition(b);
      b = qr.r * qr.q;
      eigen_vectors = eigen_vectors * qr.q;

      auto off_diag_norm = frobenius_norm(b, true);
      auto norm = frobenius_norm(b);
      if (off_diag_norm / norm < convergence_tol)
	break;
    }

    size_t len = std::min(b.get_rows(), b.get_cols());
    auto eigen_values = std::make_unique<double[]>(len);

    for (int i = 0; i < len; i++) {
      eigen_values[i] = b.get(i, i);
    }
    
    return { ::std::move(eigen_values), ::std::move(eigen_vectors) };
  }

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
