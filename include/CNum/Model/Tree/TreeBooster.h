#ifndef __TREE_BOOSTER_H
#define __TREE_BOOSTER_H

#include "CNum/DataStructs/Memory/Arena.h"
#include "CNum/Model/Tree/TreeBoosterNode.h"

#include <atomic>
#include <vector>
#include <variant>
#include <future>
#include <cstring>

namespace CNum::Model::Tree {
  class TreeBoosterNode;
  struct Split;
  struct Histogram;
  struct DataPartition;
  enum split_dir;

  using DataMatrix = std::variant< CNum::DataStructs::Matrix<int>, CNum::DataStructs::Matrix<double> >;
    
  class TreeBooster {
  private:
    double predict_sample(TreeBoosterNode *node, std::span<double> &sample);

    virtual void fit_node_greedy(const CNum::DataStructs::Matrix<double> &X,
				 double *g,
				 double *h,
				 TreeBoosterNode *node,
				 int depth = 0) = 0;
    
    virtual void fit_node_hist(const CNum::DataStructs::Matrix<int> &X,
			       std::shared_ptr<struct CNum::Data::shelf[]> shelves,
			       double *g,
			       double *h,
			       DataPartition &partition,
			       const arena_view_t &parent_hist_view,
			       TreeBoosterNode *node,
			       int depth = 0) = 0;

    virtual void fit_prep(const CNum::DataStructs::Matrix<int> &X,
			  std::shared_ptr<struct CNum::Data::shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) = 0;

    virtual void fit_prep(const CNum::DataStructs::Matrix<double> &X,
			  std::shared_ptr<struct CNum::Data::shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) = 0;
    
    void destruct(TreeBoosterNode *node);
  
  public:
    TreeBoosterNode *_root;
    int _max_depth;
    int _min_samples;
    double _reg_lambda;
    double _gamma;
    double _weight_decay;
    
    arena_t *arena;

    TreeBooster(arena_t *a = nullptr,
		int md = 5,
		int ms = 3,
		double weight_decay = 0.0,
		double reg_lambda = 1.0,
		double gamma = 0.0);
    ~TreeBooster();

    virtual void fit(DataMatrix &X,
		     std::shared_ptr<struct CNum::Data::shelf[]> shelves,
		     double *g,
		     double *h,
		     DataPartition &partition) = 0;
    CNum::DataStructs::Matrix<double> predict(CNum::DataStructs::Matrix<double> &data);

    static size_t partition_data(const CNum::DataStructs::Matrix<int> &X,
				 double *g,
				 double *h,
				 size_t feat,
				 uint8_t bin,
				 const DataPartition &partition);
    static void histogram_subtraction(const arena_view_t &parent_hist_view,
				      arena_view_t &small_hist_view,
				      arena_view_t &large_hist_view);
    arena_view_t init_hist_view(size_t n_data_cols);

    std::string to_json();
  };
};
  
#endif
