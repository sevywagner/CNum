#include "CNum/Model/Tree/TreeBoosterNode.h"

using namespace CNum::DataStructs;

namespace CNum::Model::Tree {
  TreeBoosterNode::TreeBoosterNode(TreeBoosterNode *left,
				   TreeBoosterNode *right)
    : _left(left), _right(right) {

    _split.best_gain = 0.0;
    _split.feature = -1;
  }

  TreeBoosterNode::~TreeBoosterNode() {}

  Split TreeBoosterNode::find_best_split_greedy(Matrix<double> &X,
					        const double *g,
						const double *h,
						double weight_decay,
						double reg_lambda,
						double gamma) {
    
    // Exact greedy method coming soon
    return {};
  }

  
  Split TreeBoosterNode::find_best_split_hist(const Matrix<int> &X,
					      std::shared_ptr<CNum::Data::Shelf[]> shelves,
					      const double *g,
					      const double *h,
					      bool histogram_cache,
					      const arena_view_t &hist_view,
					      DataPartition &partition,
					      double weight_decay,
					      double reg_lambda,
					      double gamma) {
    constexpr uint8_t n_threads = 32;
    size_t n_features = X.get_rows();
    size_t features_per_thread = (n_features + n_threads - 1) / n_threads;
    
    auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();
    ::std::vector< ::std::future<Split> > futures;
    futures.reserve(n_threads);

    size_t *indeces = (size_t *) partition.global_idx_array->ptr;

    double gs = ::std::reduce(g + partition.start, g + partition.end);
    double hs = ::std::reduce(h + partition.start, h + partition.end);
    
    for (uint8_t thread_num = 0; thread_num < n_threads; thread_num++) {
      futures.push_back(tp->submit< Split >([&, thread_num] (arena_t *arena) {
	size_t start = thread_num * features_per_thread;
	size_t end = ::std::min(n_features, start + features_per_thread);

	Split s{ -1, 0.0, 0.0, 0, { 0.0, 0.0 } };
	
	for (int i = start; i < end; i++) {
	  Histogram hist = ((Histogram *) hist_view.ptr)[i];
	  double *g_bin = (double *) hist.g_bin.ptr;
	  double *h_bin = (double *) hist.h_bin.ptr;

	  auto row = X.get_row_view(i);

	  // if we are using a cached histogram we do not need to build the histogram
	  if (!histogram_cache) {
	    for (int j = partition.start; j < partition.end; j++) {	    
	      int b = row[indeces[j]];

	      g_bin[b] += g[j];
	      h_bin[b] += h[j];
	    }
	  }

	  double gl{ 0.0 }, hl{ 0.0 };
	  double c1{ 0.0 }, c2{ 0.0 };
	
	  // want to make splits between bins so we skip last (N_BINS - 1)
	  for (int j = 0; j < N_BINS - 1; j++) {
	    // kahan sum for numerical stability
	    double y = g_bin[j] - c1;
	    double t = gl + y;
	    c1 = (t - gl) - y;
	    gl = t;

	    y = h_bin[j] - c2;
	    t = hl + y;
	    c2 = (t - hl) - y;
	    hl = t;
        
	    double gr = gs - gl;
	    double hr = hs - hl;

	    // want to make splits between bins so we skip first
	    if (j == 0) 
	      continue;

	    if (hl < weight_decay || hr < weight_decay)
	      continue;

	    double gain = TreeBoosterNode::get_gain(gs, hs, gl, hl, gr, hr, reg_lambda, gamma);
	    if (gain < gamma) {
	      continue;
	    }
	
	    if (gain > gamma && gain > s.best_gain) {
	      s.best_gain = gain;
	      s.feature = i;
	      s.threshold = shelves[i].ranges[j];
	      s.bin = j;

	      s.values.first = -gl / (hl + reg_lambda);
	      s.values.second = -gr / (hr + reg_lambda);
	    
	    }
	  }
	}
	return s;
      }));
    }

    return TreeBoosterNode::split_comparison(futures);
  }

  Split TreeBoosterNode::split_comparison(::std::vector< ::std::future<Split> > &splits) {
    Split best_split{ -1, 0, 0 };
    for (auto &future: splits) {
      Split s = future.get();
      if (s.best_gain > best_split.best_gain)
	best_split = s;
    }

    return best_split;
  }

  double TreeBoosterNode::get_gain(double gs, double hs, double gl, double hl, double gr, double hr, double reg_lambda, double gamma) {
    return .5 * (
		 ((gl * gl) / (hl + reg_lambda)) +
		 ((gr * gr) / (hr + reg_lambda)) -
		 ((gs * gs) / (hs + reg_lambda))
		 );
  }

  ::std::string TreeBoosterNode::to_json_string() {
    ::std::string json_str("{\"split\": {");
    json_str += "\"feature\":" + ::std::to_string(_split.feature) + ",";
    json_str += "\"threshold\":" + ::std::to_string(_split.threshold) + "},";
    json_str += "\"value\":" + ::std::to_string(_value) + ",";
    json_str += "\"left\":" + (_left != nullptr ? _left->to_json_string() + "," : "{},");
    json_str += "\"right\":" + (_right != nullptr ? _right->to_json_string() : "{}");
    json_str += "}";

    return json_str;
  }
}
