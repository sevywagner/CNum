#include "CNum/Utils/ModelUtils.h"

using namespace CNum::DataStructs;
using namespace CNum::Model::Tree;

namespace CNum::Utils::ModelUtils {
  ::std::unique_ptr< Matrix<double>[] > train_test_split(const Matrix<double> &X,
							 const Matrix<double> &y,
							 double test_percentage,
							 bool shuffle) {
    auto res = ::std::make_unique< Matrix<double>[] >(4);
  
    Matrix<double> x_use;
    Matrix<double> y_use;
    int train_len = floor(X.get_rows() * (1 - test_percentage));
    int test_len = X.get_rows() - train_len;
  
    if (shuffle) {
      auto mask = std::make_unique<uint32_t[]>(X.get_rows());

      std::iota(mask.get(), mask.get() + X.get_rows(), 0);
      std::shuffle(mask.get(), mask.get() + X.get_rows(), std::mt19937{std::random_device{}()});

      Mask<IDX, uint32_t> m(X.get_rows(), 0, std::move(mask));
    
      x_use = X[m];
      y_use = y[m];
    } else {
      x_use = X;
      y_use = y;
    }

    auto train_mask_ptr = std::make_unique<uint32_t[]>(train_len);
    std::iota(train_mask_ptr.get(), train_mask_ptr.get() + train_len, 0);
    Mask<IDX, uint32_t> train_mask(train_len, 0, std::move(train_mask_ptr));

    auto test_mask_ptr = std::make_unique<uint32_t[]>(test_len);
    std::iota(test_mask_ptr.get(), test_mask_ptr.get() + test_len, train_len);
    Mask<IDX, uint32_t> test_mask(test_len, 0, std::move(test_mask_ptr));

    res[0] = x_use[train_mask];
    res[1] = x_use[test_mask];
    res[2] = y_use[train_mask];
    res[3] = y_use[test_mask];

    return res;
  }

  void print_tree(TreeBoosterNode *node, int spaces) {
    if (node->_left != nullptr) {
      print_tree(node->_left, spaces + 10);
    }

    for (int i = 10; i < spaces; i++) {
      std::cout << " ";
    }
    std::cout << node->_value << std::endl;

    if (node->_right != nullptr) {
      print_tree(node->_right, spaces + 10);
    }
  }
};
