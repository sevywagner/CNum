#include "CNum/Model/Tree/XGTreeBooster.h"

using namespace CNum::DataStructs;

namespace CNum::Model::Tree {
  
  XGTreeBooster::XGTreeBooster(arena_t *a,
			       int md,
			       int ms,
			       double weight_decay,
			       double reg_lambda,
			       double gamma)
    : TreeBooster(a, md, ms, weight_decay, reg_lambda, gamma) {
  }

  
  XGTreeBooster::~XGTreeBooster() {}

  
  void XGTreeBooster::fit_node_greedy(const Matrix<double> &X,
				      double *g,
				      double *h,
				      TreeBoosterNode *node,
				      int depth) {
    // Exact greedy method coming soon
  }

  
  void XGTreeBooster::fit_node_hist(const Matrix<int> &X,
				    std::shared_ptr<CNum::Data::Shelf[]> shelves,
				    double *g,
				    double *h,
				    DataPartition &partition,
				    const arena_view_t &parent_hist_view,
				    TreeBoosterNode *node,
				    int depth) {
    
    if (depth >= _max_depth || partition.end - partition.start < _min_samples || node->_split.feature == -1) {
      return;
    }
    
    arena_view_t small_hist_view = TreeBooster::init_hist_view(X.get_rows());
    arena_view_t large_hist_view = parent_hist_view;

    // partition data based on split
    size_t mid_point = TreeBooster::partition_data(::std::cref(X), g, h,
						  node->_split.feature,
						  node->_split.bin,
						  ::std::cref(partition));

    if (mid_point == partition.start || mid_point == partition.end) { // if the left or right side has 0 samples
      return;
    }

    size_t left_pos_ct = mid_point - partition.start;
    size_t right_pos_ct = partition.end - mid_point;

    DataPartition left_partition{ partition.global_idx_array,
				  partition.start,
				  mid_point };

    DataPartition right_partition{ partition.global_idx_array,
				   mid_point,
				   partition.end };

    auto *left_subtree = new TreeBoosterNode();
    auto *right_subtree = new TreeBoosterNode();

    auto values = ::std::move(node->_split.values);
    left_subtree->_value = values.first;
    right_subtree->_value = values.second;

    enum split_dir small_side = left_pos_ct <= right_pos_ct ? LEFT : RIGHT;
    
    auto split_small = TreeBoosterNode::find_best_split_hist(::std::cref(X), shelves, g, h,
							     false,
							     small_hist_view,
							     small_side == LEFT ? left_partition : right_partition,
							     TreeBooster::_weight_decay,
							     TreeBooster::_reg_lambda,
							     TreeBooster::_gamma);

    // histogram caching
    TreeBooster::histogram_subtraction(::std::cref(parent_hist_view),
				       ::std::ref(small_hist_view),
				       ::std::ref(large_hist_view));

    auto split_large = TreeBoosterNode::find_best_split_hist(::std::cref(X), shelves, g, h,
							     true,
							     large_hist_view,
							     small_side == RIGHT ? left_partition : right_partition,
							     TreeBooster::_weight_decay,
							     TreeBooster::_reg_lambda,
							     TreeBooster::_gamma);
    
    if (small_side == LEFT) {
      left_subtree->_split = split_small;
      right_subtree->_split = split_large;
    } else {
      left_subtree->_split = split_large;
      right_subtree->_split = split_small;
    }

    fit_node_hist(X, shelves, g, h,
		  left_partition,
		  small_side == LEFT ? small_hist_view : large_hist_view,
		  left_subtree,
		  depth + 1);
    
    fit_node_hist(X, shelves, g, h,
		  right_partition,
		  small_side == RIGHT ? small_hist_view : large_hist_view,
		  right_subtree,
		  depth + 1);
  
    node->_left = left_subtree;
    node->_right = right_subtree;
  }

  
  void XGTreeBooster::fit_prep(const Matrix<double> &X,
			       std::shared_ptr<CNum::Data::Shelf[]> shelves,
			       double *g,
			       double *h,
			       DataPartition &partition) {
    fit_node_greedy(::std::ref(X), g, h, _root);
  }

  void XGTreeBooster::fit_prep(const Matrix<int> &X,
			       std::shared_ptr<CNum::Data::Shelf[]> shelves,
			       double *g,
			       double *h,
			       DataPartition &partition) {
    arena_view_t hist_view = TreeBooster::init_hist_view(X.get_rows());
    
    auto split = TreeBoosterNode::find_best_split_hist(::std::cref(X),
						       shelves,
						       g,
						       h,
						       false,
						       hist_view,
						       ::std::ref(partition),
						       TreeBooster::_weight_decay,
						       TreeBooster::_reg_lambda,
						       TreeBooster::_gamma);

    double gs = ::std::reduce(g + partition.start, g + partition.end);
    double hs = ::std::reduce(h + partition.start, h + partition.end);
	
    _root->_split = split;
    _root->_value = -gs / (hs + TreeBooster::_reg_lambda);
    
    fit_node_hist(::std::cref(X),
		  shelves,
		  g,
		  h,
		  ::std::ref(partition),
		  hist_view,
		  _root);
    
  }

  void XGTreeBooster::fit(DataMatrix &X,
			  std::shared_ptr<CNum::Data::Shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) {
    ::std::visit([&, this] (auto &x) {
      _root = new TreeBoosterNode();
      fit_prep(::std::cref(x), shelves, g, h, ::std::ref(partition));
    }, X);
  }
};
