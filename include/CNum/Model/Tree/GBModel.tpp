// ------------------------------
// Constructors and destructors
// ------------------------------

// Overloaded constructor
template <typename TreeType>
GBModel<TreeType>::GBModel(::std::string lt,
			   int n_learners,
			   double lr,
			   double ss,
			   int md,
			   int ms,
			   enum split_alg sa,
			   ::std::string activation_func,
			   double weight_decay,
			   double rl,
			   double gamma)
  : _loss_type(lt),
    _n_learners(n_learners),
    _learning_rate(lr),
    _subsample(ss),
    _max_depth(md),
    _min_samples(ms),
    _sa(sa),
    _weight_decay(weight_decay),
    _activation(activation_func),
    _reg_lambda(rl),
    _gamma(gamma) {
  _trees = new TreeType[n_learners];
}

// ---- Destructor ----
template <typename TreeType>
GBModel<TreeType>::~GBModel() {
  delete[] _trees;
}

// ------------------------
// Training and inference
// ------------------------

// ---- Train Model ----
template <typename TreeType>
void GBModel<TreeType>::fit(CNum::DataStructs::Matrix<double> &X,
			    CNum::DataStructs::Matrix<double> &y) {
  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();

  auto a = tp->submit< void >([&, this] (arena_t *arena) {
    ::std::shared_ptr<struct CNum::Data::shelf[]> shelves = _sa == GREEDY ? nullptr : CNum::Data::quantile_bin(::std::cref(X), N_BINS);

      DataMatrix data = apply_quantile(::std::cref(X), shelves).transpose();

      CNum::DataStructs::Matrix<double> fm = CNum::DataStructs::Matrix<double>::init_const(y.get_rows(), 1, 0);
      auto *loss = Loss::get_loss_obj();

      ::std::visit([&, this] (auto &x) {
	using T = ::std::decay_t<decltype(x)>;
      
	size_t n_samples = ::std::min(static_cast<int>(_subsample * X.get_rows()),
				      static_cast<int>(X.get_rows()));
      
	for (int i = 0; i < _n_learners; i++) {
	  arena_view_t position_array = arena_malloc(arena, sizeof(size_t) * n_samples, sizeof(size_t));
	  arena_view_t g_sub = arena_malloc(arena, sizeof(double) * n_samples, sizeof(double));
	  arena_view_t h_sub = arena_malloc(arena, sizeof(double) * n_samples, sizeof(double));
	
	  size_t *pos_ptr = (size_t *) position_array.ptr;
	  double *g_sub_ptr = (double *) g_sub.ptr;
	  double *h_sub_ptr = (double *) h_sub.ptr;

	  if (_subsample == 1.0) {
	    ::std::iota(pos_ptr, pos_ptr + n_samples, 0);
	  } else {
	    CNum::Utils::Rand::generate_n_unique_rand_in_range<size_t>(0,
								       X.get_rows() - 1,
								       pos_ptr,
								       n_samples);
	  }
     
	  DataPartition partition{ &position_array, 0, n_samples };

	  loss->get_gradients_hessians(y, fm, g_sub, h_sub, position_array, _loss_type);

	  _trees[i] = TreeType(arena,
			       _max_depth,
			       _min_samples,
			       _weight_decay,
			       _reg_lambda,
			       _gamma);

	
        
	  _trees[i].fit(data, shelves, g_sub_ptr, h_sub_ptr, partition);
	  fm = fm + (_trees[i].predict(X) * _learning_rate);
      
	  if (i % 5 == 0) {
	    ::std::cout << "[*] Learner #" << i << " loss: "
			<< loss->get_loss(y, fm, _loss_type) << ::std::endl;
	  }

	  arena_clear(arena);
	}
      }, data);
  });

  a.wait();
}

// ---- Inference ----
template <typename TreeType>
CNum::DataStructs::Matrix<double> GBModel<TreeType>::predict(CNum::DataStructs::Matrix<double> &data) {
  auto preds = CNum::DataStructs::Matrix<double>::init_const(data.get_rows(), 1, 0);
  
  ::std::for_each(_trees, _trees + _n_learners, [&] (TreeBooster &t) {
    auto t_preds = t.predict(data);
    preds = preds + (t_preds * _learning_rate);
  });

  if (_activation.size() > 0) {
    auto *act = Activation::get_activation_obj();
    preds = act->activate(preds, _activation);
  }
  
  return preds;
}

// ----------------------------
// Saving and loading models
// ----------------------------

// ---- Save Model to JSON encoded ".cmod" file ----
template <typename TreeType>
void GBModel<TreeType>::save_model(::std::string path) {
  ::std::string json_str("{\"loss_type\":\"");
  json_str += _loss_type + "\",";
  json_str += "\"learning_rate\":" + ::std::to_string(_learning_rate) + ",";
  json_str += "\"n_learners\":" + ::std::to_string(_n_learners) + ",";
  json_str += "\"subsample\":" + ::std::to_string(_subsample) + ",";
  json_str += "\"max_depth\":" + ::std::to_string(_max_depth) + ",";
  json_str += "\"min_samples\":" + ::std::to_string(_min_samples) + ",";
  json_str += "\"activation\":\"" + _activation + "\",";
  json_str += "\"weight_decay\":" + ::std::to_string(_weight_decay) + ",";
  json_str += "\"reg_lambda\":" + ::std::to_string(_reg_lambda) + ",";
  json_str += "\"gamma\":" + ::std::to_string(_gamma) + ",";
  
  json_str += "\"learners\": [";

  for (int i = 0; i < _n_learners; i++) {
    json_str += _trees[i].to_json() + ",";
  }

  json_str.pop_back();
  json_str += "]}";
  
  ::std::ofstream of(path);

  if (!of.is_open()) {
    ::std::runtime_error("GB model saving error - Error opening file");
  }

  of << json_str;
}

// ---- Parse singular json learner ----
template <typename TreeType>
TreeBoosterNode *GBModel<TreeType>::parse_learner(json node) {
  auto *res = new TreeBoosterNode();
  
  res->_split = { node["split"]["feature"], node["split"]["threshold"], 1e-4, 0, { 0, 0 } };
  res->_value = node["value"];
  res->_left = node["left"].dump() == "{}" ? nullptr : parse_learner(node["left"]);
  res->_right = node["right"].dump() == "{}" ? nullptr : parse_learner(node["right"]);

  return res;
}

#include "CNum/Utils/ModelUtils.h"

// ---- Load Model from JSON encoded ".cmod" file ----
template <typename TreeType>
GBModel<TreeType> GBModel<TreeType>::load_model(::std::string path) {
  ::std::ifstream is(path);

  if (!is.is_open()) {
    ::std::runtime_error("GB model loading error - file could not be opened");
  }

  json data = json::parse(is);
  GBModel<TreeType> res(data["loss_type"],
			data["n_learners"],
			data["learning_rate"],
			data["subsample"],
			data["max_depth"],
			data["min_samples"],
			HIST,
			data["activation"],
			data["weight_decay"],
			data["reg_lambda"],
			data["gamma"]);

  auto learners = data["learners"];
  int ctr = 0;
  for (auto &[key, value]: learners.items()) {
    auto *tree = GBModel<TreeType>::parse_learner(value);
    res._trees[ctr]._root = ::std::exchange(tree, nullptr);
    ctr++;
  }
  
  return res;
}
