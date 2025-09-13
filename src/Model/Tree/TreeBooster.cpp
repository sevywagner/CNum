#include "CNum/Model/Tree/TreeBooster.h"

  // -----------
  // Tree
  // -----------

using namespace CNum::DataStructs;

namespace CNum::Model::Tree {
  // ------------------------------
  // Constructors and destructors
  // ------------------------------
  
  // ---- Overloaded constructer -----
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
      arena(a) {}

  // ---- Recursively free nodes ----
  void TreeBooster::destruct(TreeBoosterNode *node) {
    if (node == nullptr)
      return;

    destruct(node->_left);
    destruct(node->_right);
    delete node;
  }

  // ---- Destructor ----
  TreeBooster::~TreeBooster() {
    this->destruct(_root);
  }

  // -------------
  // Inference
  // -------------

  // ---- Inference on a matrix ----
  Matrix<double> TreeBooster::predict(Matrix<double> &data) {
    int n_samples = data.get_rows();
    auto pred_ptr = ::std::make_unique<double[]>(data.get_rows());

    for (int i = 0; i < n_samples; i++) {
      auto sample = data.get_row_view(i);
      pred_ptr[i] = predict_sample(_root, ::std::ref(sample));
    }
  
    return Matrix<double>(data.get_rows(), 1, ::std::move(pred_ptr));
  }

  // ---- Inference on a single sample ----
  double TreeBooster::predict_sample(TreeBoosterNode *node, std::span<double> &sample) {
    if (node->_right == nullptr || node->_left == nullptr || node->_split.feature == -1)
      return node->_value;

    if (sample[node->_split.feature] <= node->_split.threshold)
      return predict_sample(node->_left, ::std::ref(sample));
    else if (sample[node->_split.feature] > node->_split.threshold)
      return predict_sample(node->_right, ::std::ref(sample));

    return node->_value;
  }

  // -----------
  // Utils
  // -----------

  // ---- Allocate space for histograms on the arena ----
  arena_view_t TreeBooster::init_hist_view(size_t n_data_cols) {
    arena_view_t hist_view = arena_malloc(arena, sizeof(Histogram) * n_data_cols, sizeof(Histogram));

    Histogram *histograms = (Histogram *) hist_view.ptr;
    for (int i = 0; i < n_data_cols; i++) {
      arena_view_t g_bin_view = arena_malloc(arena, sizeof(double) * N_BINS, sizeof(double));
      arena_view_t h_bin_view = arena_malloc(arena, sizeof(double) * N_BINS, sizeof(double));
      
      histograms[i] = { g_bin_view, h_bin_view };
    }
    
    return hist_view;
  }

  // ---- Partition idx array, g, and h to make node slices contigous ----
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

  // ---- Subtract parent hist from small hist for histogram caching ----
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

  // ---- Save tree data in json encoded string ----
  ::std::string TreeBooster::to_json() {
    return _root->to_json_string();
  }
};
