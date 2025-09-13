#ifndef __TREE_BOOSTER_NODE_H
#define __TREE_BOOSTER_NODE_H

#include "CNum/DataStructs/DataStructs.h"
#include "CNum/Data/Data.h"

#include <vector>
#include <future>
#include <cstring>

namespace CNum::Model::Tree {
  constexpr int N_BINS = 256;
  
  using SplitValuePair = std::pair<double, double>;
  struct DataPartition;

  struct Histogram {
    arena_view_t g_bin;
    arena_view_t h_bin;
  };

  struct DataPartition {
    arena_view_t *global_idx_array;
    size_t start;
    size_t end;
  };
  
  struct Split {
    int feature;
    double threshold;
    double best_gain;
    int bin;
    SplitValuePair values;
  };

  enum split_dir {
    LEFT,
    RIGHT
  };

  // -------------
  // Tree Node
  // -------------
  
  class TreeBoosterNode {
  private:
    static double get_gain(double gs,
			   double hs,
			   double gl,
			   double hl,
			   double gr,
			   double hr,
			   double reg_lambda = 1.0,
			   double gamma = 0);

    static Split split_comparison(std::vector< std::future<Split> > &splits);
  
  public:
    Split _split;
    double _value;
    CNum::DataStructs::Mask<CNum::DataStructs::IDX, uint32_t> _idx;
    TreeBoosterNode *_left;
    TreeBoosterNode *_right;

    TreeBoosterNode(TreeBoosterNode *left = nullptr,
		    TreeBoosterNode *right = nullptr);
    
    virtual ~TreeBoosterNode();

    static Split find_best_split_hist(const CNum::DataStructs::Matrix<int> &X,
				      std::shared_ptr<struct CNum::Data::shelf[]> shelves,
				      const double *g,
				      const double *h,
				      bool histogram_cache,
				      const arena_view_t &hist_view,
				      DataPartition &partition,
				      double weight_decay = 0.0,
				      double reg_lambda = 1.0,
				      double gamma = 0);
    
    static Split find_best_split_greedy(CNum::DataStructs::Matrix<double> &X,
				        const double *g,
				        const double *h,
					double weight_decay = 0.0,
					double reg_lambda = 1.0,
					double gamma = 0);
    
    std::string to_json_string();

    friend class TreeBooster;
  };
};
  
#endif
