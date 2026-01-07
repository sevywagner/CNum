#include "CNum/Model/Tree/TreeBooster.h"
#include "CNum/Model/Tree/Tree.h"

  // -----------
  // Tree
  // -----------

using namespace CNum::DataStructs;

namespace CNum::Model::Tree {
  struct Histogram;
  class TreeBoosterNode;
  
  // ------------------------------
  // Constructors and destructors
  // ------------------------------
  
  TreeBooster::TreeBooster(arena_t *a,
			   int md,
			   int ms,
			   double weight_decay,
			   double reg_lambda,
			   double gamma)
    : _max_depth(md),
      _min_samples(ms),
      _weight_decay(weight_decay),
      _reg_lambda(reg_lambda),
      _gamma(gamma),
      _root(nullptr),
      _arena(a) {}

  
  void TreeBooster::copy_hyperparams(const TreeBooster &other) noexcept {
    this->_max_depth = other._max_depth;
    this->_min_samples = other._min_samples;
    this->_reg_lambda = other._reg_lambda;
    this->_gamma = other._gamma;
    this->_weight_decay = other._weight_decay;
  }
  
  
  void TreeBooster::move(TreeBooster &&other) noexcept {
    if (this == &other) return;
    this->copy_hyperparams(other);
    
    if (this->_root != nullptr) destruct(_root);
    this->_arena = ::std::exchange(other._arena, nullptr);
    this->_root = ::std::exchange(other._root, nullptr);
  }

  TreeBoosterNode *TreeBooster::copy_tree(TreeBoosterNode *node) noexcept {
    if (node == nullptr) return nullptr;

    TreeBoosterNode *cp_node = new TreeBoosterNode();
    cp_node->_split = node->_split;
    cp_node->_value = node->_value;
    
    cp_node->_left = copy_tree(node->_left);
    cp_node->_right = copy_tree(node->_right);
    return cp_node;
  }
  
  
  void TreeBooster::copy(const TreeBooster &other) noexcept {
    if (this == &other) return;
    this->copy_hyperparams(other);
 
    if (this->_root != nullptr)
      destruct(_root);
    
    this->_arena = other._arena;
    this->_root = copy_tree(other._root);
  }

  
  TreeBooster::TreeBooster(const TreeBooster &other) noexcept {
    this->copy(other);
  }
  
  
  TreeBooster &TreeBooster::operator=(const TreeBooster &other) noexcept {
    this->copy(other);
    return *this;
  }
  
  
  TreeBooster::TreeBooster(TreeBooster &&other) noexcept {
    this->move(::std::move(other)); 
  }
  
  
  TreeBooster &TreeBooster::operator=(TreeBooster &&other) noexcept {
    this->move(::std::move(other));
    return *this;
  }

  
  void TreeBooster::destruct(TreeBoosterNode *node) {
    if (node == nullptr)
      return;

    destruct(node->_left);
    destruct(node->_right);
    delete node;
  }

  
  TreeBooster::~TreeBooster() {
    this->destruct(_root);
  }

  void TreeBooster::set_root(TreeBoosterNode *root) {
    if (this->_root != nullptr)
      destruct(_root);

    _root = root;
  }

  // -------------
  // Inference
  // -------------

  
  Matrix<double> TreeBooster::predict(Matrix<double> &data) {
    int n_samples = data.get_rows();
    auto pred_ptr = ::std::make_unique<double[]>(data.get_rows());

    for (int i = 0; i < n_samples; i++) {
      auto sample = data.get_row_view(i);
      pred_ptr[i] = predict_sample(_root, sample);
    }
  
    return Matrix<double>(data.get_rows(), 1, ::std::move(pred_ptr));
  }

  /// @brief Inference (make predictions) on a single sample
  /// @param node A node in the TreeBooster
  /// @param sample The sample to make predictions on
  /// @return The prediction
  double TreeBooster::predict_sample(TreeBoosterNode *node, std::span<double> &sample) {
    if (node->_right == nullptr || node->_left == nullptr || node->_split.feature == -1)
      return node->_value;

    if (sample[node->_split.feature] <= node->_split.threshold)
      return predict_sample(node->_left, sample);
    else if (sample[node->_split.feature] > node->_split.threshold)
      return predict_sample(node->_right, sample);

    return node->_value;
  }

  // -----------
  // Utils
  // -----------

  
  arena_view_t TreeBooster::init_hist_view(size_t n_data_cols) {
    arena_view_t hist_view = arena_malloc(_arena, sizeof(Histogram) * n_data_cols, sizeof(Histogram));

    Histogram *histograms = (Histogram *) hist_view.ptr;
    for (int i = 0; i < n_data_cols; i++) {
      arena_view_t g_bin_view = arena_malloc(_arena, sizeof(double) * N_BINS, sizeof(double));
      arena_view_t h_bin_view = arena_malloc(_arena, sizeof(double) * N_BINS, sizeof(double));
      
      histograms[i] = { g_bin_view, h_bin_view };
    }
    
    return hist_view;
  }

  
  size_t TreeBooster::partition_data(const Matrix<int> &X,
				     double *g,
				     double *h,
				     size_t feat,
				     uint8_t bin,
				     const DataPartition &partition) {
    size_t *indeces = (size_t *) partition.global_idx_array->ptr;
    size_t *l_idx_ptr = indeces + partition.start;
    size_t *r_idx_ptr = indeces + partition.end - 1;

    double *l_g_ptr = g + partition.start;
    double *r_g_ptr = g + partition.end - 1;

    double *l_h_ptr = h + partition.start;
    double *r_h_ptr = h + partition.end - 1;

    while (l_idx_ptr <= r_idx_ptr) {
      while (l_idx_ptr <= r_idx_ptr && X.get(feat, *l_idx_ptr) <= bin) {
        l_idx_ptr++;
	l_g_ptr++;
	l_h_ptr++;
      }

      while (l_idx_ptr <= r_idx_ptr && X.get(feat, *r_idx_ptr) > bin) {
        r_idx_ptr--;
	r_g_ptr--;
	r_h_ptr--;
      }

      if (l_idx_ptr <= r_idx_ptr) {
        size_t idx_temp = *l_idx_ptr;
	*l_idx_ptr = *r_idx_ptr;
	*r_idx_ptr = idx_temp;

	double gh_temp = *l_g_ptr;
	*l_g_ptr = *r_g_ptr;
	*r_g_ptr = gh_temp;

	gh_temp = *l_h_ptr;
	*l_h_ptr = *r_h_ptr;
	*r_h_ptr = gh_temp;
      }
    }

    return partition.start + (l_idx_ptr - (indeces + partition.start));
  }

  void TreeBooster::histogram_subtraction(const arena_view_t &parent_hist_view,
					  arena_view_t &small_hist_view,
					  arena_view_t &large_hist_view) {
    Histogram *parent_histograms = (Histogram *) parent_hist_view.ptr;
    Histogram *small_histograms = (Histogram *) small_hist_view.ptr;
    Histogram *large_histograms = (Histogram *) large_hist_view.ptr;
    
    for (int i = 0; i < small_hist_view.range; i++) {
      double *parent_hist_g = (double *) parent_histograms[i].g_bin.ptr;
      double *parent_hist_h = (double *) parent_histograms[i].h_bin.ptr;
      double *large_hist_g = (double *) large_histograms[i].g_bin.ptr;
      double *large_hist_h = (double *) large_histograms[i].h_bin.ptr;
      double *small_hist_g = (double *) small_histograms[i].g_bin.ptr;
      double *small_hist_h = (double *) small_histograms[i].h_bin.ptr;
      
      for (int j = 0; j < N_BINS; j++) {
	large_hist_g[j] = parent_hist_g[j] - small_hist_g[j];
        large_hist_h[j] = parent_hist_h[j] - small_hist_h[j];
      }
    }
  }

  
  ::std::string TreeBooster::to_json() {
    return _root->to_json_string();
  }
};
