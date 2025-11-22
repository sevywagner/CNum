#ifndef TREE_BOOSTER_H
#define TREE_BOOSTER_H

#include "CNum/DataStructs/DataStructs.h"
#include "CNum/Data/Data.h"
#include "CNum/Model/Tree/TreeDefs.h"

#include <atomic>
#include <vector>
#include <variant>
#include <future>
#include <cstring>
#include <utility>

namespace CNum::Model::Tree {
  /**
   * @class TreeBooster
   * @brief A decision tree used in various gradient-boosting models as a
   * weak learner
   *
   * The TreeBooster class is a robust and effecient weak learner that is good 
   * at recognizing subtle relationships between features in tabular data. While
   * weak on their own when used in gradient-boosting algorithms become extremely
   * powerful. 
   */
  class TreeBooster {
  protected:
    TreeBoosterNode *_root;
    int _max_depth;
    int _min_samples;
    double _reg_lambda;
    double _gamma;
    double _weight_decay;
    arena_t *_arena;
    
  private:
    double predict_sample(TreeBoosterNode *node, std::span<double> &sample);

    virtual void fit_node_greedy(const CNum::DataStructs::Matrix<double> &X,
				 double *g,
				 double *h,
				 TreeBoosterNode *node,
				 int depth = 0) = 0;
    
    virtual void fit_node_hist(const CNum::DataStructs::Matrix<int> &X,
			       std::shared_ptr<CNum::Data::Shelf[]> shelves,
			       double *g,
			       double *h,
			       DataPartition &partition,
			       const arena_view_t &parent_hist_view,
			       TreeBoosterNode *node,
			       int depth = 0) = 0;

    virtual void fit_prep(const CNum::DataStructs::Matrix<int> &X,
			  std::shared_ptr<CNum::Data::Shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) = 0;

    virtual void fit_prep(const CNum::DataStructs::Matrix<double> &X,
			  std::shared_ptr<CNum::Data::Shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) = 0;
    /// @brief Recursively free nodes
    void destruct(TreeBoosterNode *node);

    /// @brief Copy the hyperparameters from one TreeBooster to another
    void copy_hyperparams(const TreeBooster &other) noexcept;
    
    /// @brief The move logic
    void move(TreeBooster &&other) noexcept;

    /// @brief Make a copy of a TreeBooster
    /// @return The copy of the TreeBooster
    TreeBoosterNode *copy_tree(TreeBoosterNode *node) noexcept;

    /// @brief The copy logic
    void copy(const TreeBooster &other) noexcept;
  
  public:
    /// @brief Overloaded default constructor
    /// @param a The arena to use for allocation in this tree (arena of the parent
    /// thread in the tree building process)
    TreeBooster(arena_t *a = nullptr,
		int md = 5,
		int ms = 3,
		double weight_decay = 0.0,
		double reg_lambda = 1.0,
		double gamma = 0.0);


    /// @brief Copy constructor
    TreeBooster(const TreeBooster &other) noexcept;

    /// @brief Copy equals operator
    TreeBooster &operator=(const TreeBooster &other) noexcept;

    /// @brief Move constructor
    TreeBooster(TreeBooster &&other) noexcept;

    /// @brief Move equals operator
    TreeBooster &operator=(TreeBooster &&other) noexcept;

    /// @brief Destructor
    virtual ~TreeBooster();

    /// @brief Set the root of the tree
    void set_root(TreeBoosterNode *root);

    virtual void fit(DataMatrix &X,
		     std::shared_ptr<CNum::Data::Shelf[]> shelves,
		     double *g,
		     double *h,
		     DataPartition &partition) = 0;

    /// @brief Inference (making predictions) on tabular data
    /// @param data The data to make predictions on
    /// @return The predictions
    CNum::DataStructs::Matrix<double> predict(CNum::DataStructs::Matrix<double> &data);

    /// @brief Partition idx array, g, and h based on a split to make 
    /// each nodes' slice of the dataset contigous
    /// @param X The dataset (row-wise features)
    /// @param g The gradient array
    /// @param h The hessian array
    /// @param feat The feature associated with the split
    /// @param bin The bin associated with the split
    /// @param partition The current node's data partition
    /// @return The index of the boundary between the left and right partitions
    static size_t partition_data(const CNum::DataStructs::Matrix<int> &X,
				 double *g,
				 double *h,
				 size_t feat,
				 uint8_t bin,
				 const DataPartition &partition);

    /// @brief Subtract a parent histogram from "small" histogram for histogram caching
    ///
    /// CNum's tree boosting models exploit histogram caching which reduces the amount
    /// of histograms built by building the histogram for the smaller of 2 child
    /// node partitions and subtracting it from the parent to yield the larger
    /// partition's histogram
    /// @param parent_hist_view The arena_view_t with the parent histogram
    /// @param small_hist_view The arena_view_t with the small partition's histogram
    /// @param large_hist_view The arena_view_t that we will fill with the difference
    /// of the parent and small histograms
    static void histogram_subtraction(const arena_view_t &parent_hist_view,
				      arena_view_t &small_hist_view,
				      arena_view_t &large_hist_view);

    /// @brief Allocate space for histograms on the arena
    /// @param n_data_cols The number of features in the dataset
    /// @return An arena_view_t with the histograms
    arena_view_t init_hist_view(size_t n_data_cols);

    /// @brief Save tree data in json encoded string
    /// @return The JSON string
    std::string to_json();
  };
};
  
#endif
