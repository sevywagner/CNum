#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include "CNum/DataStructs/DataStructs.h"
#include "CNum/Model/Tree/Tree.h"
#include "CNum/Utils/Utils.h"

/**
 * @namesapce CNum::Utils::ModelUtils
 * @brief Utilities used in CNum ML models
 */
namespace CNum::Utils::ModelUtils {
  /// @brief Split a data set into train and split subsets
  /// @param X The dataset
  /// @param y The labels for the dataset
  /// @param test_percentage The percentage of the dataset to use for the test
  /// subset
  /// @param shuffle Whether or not to shuffle the dataset
  ::std::unique_ptr<CNum::DataStructs::Matrix<double>[] > train_test_split(const CNum::DataStructs::Matrix<double> &X,
						      const CNum::DataStructs::Matrix<double> &y,
						      double test_percentage = 0.2,
						      bool shuffle = true);

  /// @brief Print a tree (left to right instead of top down)
  /// @param node A node in a TreeBooster
  /// @param spaces the amount of spaces to print before the node's
  /// value
  void print_tree(CNum::Model::Tree::TreeBoosterNode *node, int spaces = 10);
};

#endif
