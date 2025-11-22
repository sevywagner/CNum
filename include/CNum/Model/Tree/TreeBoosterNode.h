#ifndef TREE_BOOSTER_NODE_H
#define TREE_BOOSTER_NODE_H

#include "CNum/DataStructs/DataStructs.h"
#include "CNum/Data/Data.h"
#include "CNum/Model/Tree/TreeDefs.h"

#include <vector>
#include <future>
#include <cstring>

namespace CNum::Model::Tree {
  struct Split;
  struct DataPartition;

  // -------------
  // Tree Node
  // -------------
  
  /**
   * @class TreeBoosterNode
   * @brief A node used in a TreeBooster used for gather and storing 
   * information about the decision making process
   */
  class TreeBoosterNode {
  private:
    /// @brief Calculate the gain of a split
    ///
    /// Gain is the metric used to detirmine how good a split is in histogram tree building.
    /// The higher the gain, the better the split
    /// @param gs The sum of the gradient array
    /// @param hs The sum of the hessian array
    /// @param gl The sum of the gradient array left of the split
    /// @param hl The sum of the hessian array left of the split
    /// @param gr The sum of the gradient array right of the split
    /// @param hr The sum of the hessian array right of the split
    /// @return The gain
    static double get_gain(double gs,
			   double hs,
			   double gl,
			   double hl,
			   double gr,
			   double hr,
			   double reg_lambda = 1.0,
			   double gamma = 0);

    /// @brief Compare the gains of multiple splits and find the maximum
    /// @param splits The splits
    /// @return The best split
    static Split split_comparison(std::vector< std::future<Split> > &splits);
  
  public:
    Split _split;
    double _value;
    TreeBoosterNode *_left;
    TreeBoosterNode *_right;

    /// @brief Overloaded default constructor
    /// @param left The left child of the tree
    /// @param right The right child of the tree
    TreeBoosterNode(TreeBoosterNode *left = nullptr,
		    TreeBoosterNode *right = nullptr);
    
    virtual ~TreeBoosterNode();

    /// @brief Find the best split at a tree node with the
    /// histogram method (maximizing gain)
    /// @param X The dataset
    /// @param shelves The bins and values associated with their boundaries
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param histogram_cache Whether or not the histograms have already been built
    /// @param hist_view The view of the histograms
    /// @param partition The partition of the node's slice of the dataset
    /// @param weight_decay A parameter used in deteriming whether or not a splits
    /// effect is significant enough to take
    /// @param reg_lambda Reg Lambda; A regularization parameter
    /// @param gamma Gamma; A regularization parameter
    /// @return The best split
    static Split find_best_split_hist(const CNum::DataStructs::Matrix<int> &X,
				      std::shared_ptr<CNum::Data::Shelf[]> shelves,
				      const double *g,
				      const double *h,
				      bool histogram_cache,
				      const arena_view_t &hist_view,
				      DataPartition &partition,
				      double weight_decay = 0.0,
				      double reg_lambda = 1.0,
				      double gamma = 0);

    /// @brief Find the best split at a tree node with the exact
    /// greedy method proposed in Chen & Guestrin's XGBoost (minimizing loss)
    ///
    /// Available next release
    /// @param X The dataset
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param weight_decay A parameter used in deteriming whether or not a splits
    /// effect is significant enough to take
    /// @param reg_lambda Reg Lambda; A regularization parameter
    /// @param gamma Gamma; A regularization parameter
    /// @return The best split
    static Split find_best_split_greedy(CNum::DataStructs::Matrix<double> &X,
				        const double *g,
				        const double *h,
					double weight_decay = 0.0,
					double reg_lambda = 1.0,
					double gamma = 0);

    /// @brief Save TreeBoosterNode data for an entire TreeBooster in a JSON formatted string
    /// @return The JSON string
    std::string to_json_string();

    friend class TreeBooster;
  };
};

#endif
