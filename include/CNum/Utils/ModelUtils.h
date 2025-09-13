#ifndef __MODEL_UTILS_H
#define __MODEL_UTILS_H

#include "CNum/DataStructs/Matrix/Mask.h"
#include "CNum/DataStructs/Matrix/Matrix.h"
#include "CNum/Model/Tree/TreeBooster.h"
#include "CNum/Utils/Utils.h"

namespace CNum::Utils::ModelUtils {
  ::std::unique_ptr<CNum::DataStructs::Matrix<double>[] > train_test_split(const CNum::DataStructs::Matrix<double> &X,
						      const CNum::DataStructs::Matrix<double> &y,
						      double test_percentage = 0.2,
						      bool shuffle = true);

  void print_tree(CNum::Model::Tree::TreeBoosterNode *node, int spaces = 10);
};

#endif
