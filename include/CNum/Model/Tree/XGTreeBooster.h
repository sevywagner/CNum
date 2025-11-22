#ifndef XG_TREE_BOOSTER_H
#define XG_TREE_BOOSTER_H

#include "CNum/Data/Data.h"
#include "CNum/Model/Tree/TreeBoosterNode.h"
#include "CNum/Model/Tree/TreeBooster.h"
#include "CNum/Model/Tree/TreeDefs.h"

namespace CNum::Model::Tree {
  /**
   * @class XGTreeBooster
   * @brief A tree booster modeled after Chen & Guestrin's XGBoost tree booster
   */
  class XGTreeBooster : public TreeBooster {
  private:
    /// @brief Exact Greedy Tree Building
    virtual void fit_node_greedy(const CNum::DataStructs::Matrix<double> &X,
				 double *g,
				 double *h,
				 TreeBoosterNode *node,
				 int depth = 0) override;

    /// @brief Histogram Tree Building
    ///
    /// A recursive tree building process modeled after Chen & Guestrin's approach in XGBoost
    /// @param X The dataset
    /// @param shelves The bins and values associated with their boundaries
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param partition The partition of the node's slice of the dataset
    /// @param parent_hist_view The view of the histograms associated with
    /// node's slice of the dataset
    /// @param node The node
    /// @param depth The depth of the node
    virtual void fit_node_hist(const CNum::DataStructs::Matrix<int> &X,
			       std::shared_ptr<CNum::Data::Shelf[]> shelves,
			       double *g,
			       double *h,
			       DataPartition &partition,
			       const arena_view_t &parent_hist_view,
			       TreeBoosterNode *node,
			       int depth = 0) override;

    /// @brief Preperation for histogram tree build
    /// @param X The dataset
    /// @param shelves The bins and values associated with their boundaries
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param partition The partition of the node's slice of the dataset
    virtual void fit_prep(const CNum::DataStructs::Matrix<int> &X,
			  std::shared_ptr<CNum::Data::Shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) override;

    /// @brief Preperation for exact greedy tree build
    /// @param X The dataset
    /// @param shelves The bins and values associated with their boundaries
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param partition The partition of the node's slice of the dataset
    virtual void fit_prep(const CNum::DataStructs::Matrix<double> &X,
			  std::shared_ptr<CNum::Data::Shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) override;
  
  public:
    /// @brief Overloaded constructor
    /// @param md The max depth of the tree
    /// @param ms The min samples needed to continue with the tree building
    /// process
    /// @param reg_lambda Reg Lambda; A regularization parameter
    /// @param gamma Gamma; A regularization parameter
    XGTreeBooster(arena_t *a = nullptr,
		  int md = 5,
		  int ms = 3,
		  double weight_decay = 0.0,
		  double reg_lambda = 1.0,
		  double gamma = 0.0);

    /// @brief Destructor
    ~XGTreeBooster();

    /// @brief Unified fit function
    /// @param X The dataset
    /// @param shelves The bins and values associated with their boundaries
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param partition The partition of the node's slice of the dataset
    virtual void fit(DataMatrix &X,
		     std::shared_ptr<CNum::Data::Shelf[]> shelves,
		     double *g,
		     double *h,
		     DataPartition &partition) override;
  };
};

#endif
