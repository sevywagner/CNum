#ifndef TREE_DEFS_H
#define TREE_DEFS_H

#include "CNum/DataStructs/DataStructs.h"
#include <utility>
#include <variant>

/**
 * @namespace CNum::Model::Tree
 * @brief Tree-based models
 */
namespace CNum::Model::Tree {
  using SplitValuePair = std::pair<double, double>;
  
  /// @brief Number of bins used in the Tree models
  /// 
  /// The value 256 was chosen for vgather optimizations. If there are 256 bins
  /// then the bin number fits in one byte, and we can gather more gradients and
  /// hessians associated with bin numbers in parrallel when searching for the
  /// best split.
  constexpr int N_BINS = 256;

  using DataMatrix = std::variant< CNum::DataStructs::Matrix<int>, CNum::DataStructs::Matrix<double> >;

  /**
   * @struct Histogram
   * @brief Holds the total gradients and hessians for all bins
   */
  struct Histogram {
    arena_view_t g_bin;
    arena_view_t h_bin;
  };

  /**
   * @struct DataPartition
   * @brief A data partition for the set of samples a tree node has to work with 
   * during the tree building process
   */
  struct DataPartition {
    arena_view_t *global_idx_array;
    size_t start;
    size_t end;
  };
  
  /**
   * @struct Split
   * @brief Holds data associated with the decision making process in a 
   * TreeBoosterNode
   */
  struct Split {
    int feature;
    double threshold;
    double best_gain;
    int bin;
    SplitValuePair values;
  };

  /**
   * @enum split_dir
   * @brief Signifies the direction of a node resultant of a split in relation
   * to its parent
   */
  enum split_dir {
    LEFT,
    RIGHT
  };

  class TreeBoosterNode;
  class TreeBooster;
  class XGTreeBooster;
  template <typename T> class GBModel;
};

#endif
