// ------------------------------
// Constructors and destructors
// ------------------------------

template <typename TreeType>
GBModel<TreeType>::GBModel(::std::string lt,
			   int n_learners,
			   double lr,
			   double ss,
			   int md,
			   int ms,
			   SplitAlg sa,
			   ::std::string activation_func,
			   double weight_decay,
			   double rl,
			   double gamma,
			   SubsampleFunction ssf)
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
    _gamma(gamma),
    _subsample_function(ssf) {
  _trees = new TreeType[n_learners];
  _loss_profile = CNum::Model::Loss::get_loss_profile(lt);
  if (!_activation.empty())
    _activation_func = CNum::Model::Activation::get_activation_func(activation_func);
  else
    _activation_func = nullptr;
}

template <typename TreeType>
GBModel<TreeType>::GBModel(CNum::Model::Loss::LossProfile loss_profile,
			   int n_learners,
			   double lr,
			   double ss,
			   int md,
			   int ms,
			   SplitAlg sa,
			   CNum::Model::Activation::ActivationFunc activation_func,
			   double weight_decay,
			   double rl,
			   double gamma,
			   SubsampleFunction ssf)
  : _loss_type(""),
    _loss_profile(loss_profile),
    _n_learners(n_learners),
    _learning_rate(lr),
    _subsample(ss),
    _max_depth(md),
    _min_samples(ms),
    _sa(sa),
    _weight_decay(weight_decay),
    _activation(""),
    _reg_lambda(rl),
    _gamma(gamma),
    _subsample_function(ssf) {
  _trees = new TreeType[n_learners];
  if (activation_func)
    _activation_func = activation_func;
}

template <typename TreeType>
void GBModel<TreeType>::copy_hyperparams(const GBModel &other) noexcept {
  this->_loss_type = other._loss_type;
  this->_n_learners = other._n_learners;
  this->_learning_rate = other._learning_rate;
  this->_subsample = other._subsample;
  this->_max_depth = other._max_depth;
  this->_min_samples = other._min_samples;
  this->_sa = other._sa;
  this->_weight_decay = other._weight_decay;
  this->_activation = other._activation;
  this->_reg_lambda = other._reg_lambda;
  this->_gamma = other._gamma;
  this->_activation_func = other._activation_func;
}

template <typename TreeType>
void GBModel<TreeType>::copy(const GBModel &other) noexcept {
  if (this == &other) return;
  this->copy_hyperparams(other);

  if (this->_trees != nullptr)
    delete[] this->_trees;

  this->_trees = new TreeType[this->_n_learners];
  ::std::copy(other._trees, other._trees + other._n_learners, this->_trees);
}

template <typename TreeType>
void GBModel<TreeType>::move(GBModel &&other) noexcept {
  if (this == &other) return;
  this->copy_hyperparams(other);

  this->_trees = ::std::exchange(other._trees, nullptr);
}

template <typename TreeType>
GBModel<TreeType>::GBModel(const GBModel &other) noexcept {
  this->copy(other);
}

template <typename TreeType>
GBModel<TreeType> &GBModel<TreeType>::operator=(const GBModel &other) noexcept {
  this->copy(other);
  return *this;
}

template <typename TreeType>
GBModel<TreeType>::GBModel(GBModel &&other) noexcept {
  this->move(::std::move(other));
}

template <typename TreeType>
GBModel<TreeType> &GBModel<TreeType>::operator=(GBModel &&other) noexcept {
  this->move(::std::move(other));
  return *this;
}

template <typename TreeType>
GBModel<TreeType>::~GBModel() {
  delete[] _trees;
}

// ------------------------
// Training and inference
// ------------------------

template <typename TreeType>
void GBModel<TreeType>::fit(CNum::DataStructs::Matrix<double> &X,
			    CNum::DataStructs::Matrix<double> &y,
			    bool verbose) {
  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();

  auto a = tp->submit< void >([&, this] (arena_t *arena) {
    ::std::shared_ptr<CNum::Data::Shelf[]> shelves = _sa == GREEDY ? nullptr : CNum::Data::quantile_bin(X, N_BINS);

      DataMatrix data = apply_quantile(X, shelves).transpose();

      CNum::DataStructs::Matrix<double> fm = CNum::DataStructs::Matrix<double>::init_const(y.get_rows(), 1, 0);

      ::std::visit([&, this] (auto &x) {
	using T = ::std::decay_t<decltype(x)>;
      
	size_t n_samples = ::std::min(static_cast<size_t>(_subsample * X.get_rows()), X.get_rows());
      
	for (int i = 0; i < _n_learners; i++) {
	  arena_view_t position_array = arena_malloc(arena, sizeof(size_t) * n_samples, sizeof(size_t));
	  arena_view_t g_sub = arena_malloc(arena, sizeof(double) * n_samples, sizeof(double));
	  arena_view_t h_sub = arena_malloc(arena, sizeof(double) * n_samples, sizeof(double));
	
	  size_t *pos_ptr = (size_t *) position_array.ptr;
	  double *g_sub_ptr = (double *) g_sub.ptr;
	  double *h_sub_ptr = (double *) h_sub.ptr;

	  _subsample_function(pos_ptr, 0, X.get_rows(), n_samples, y);
     
	  DataPartition partition{ &position_array, 0, n_samples };

	  CNum::Model::Loss::get_gradients_hessians(y,
						    fm,
						    g_sub,
						    h_sub,
						    position_array,
						    _loss_profile.gradient_func,
						    _loss_profile.hessian_func);

	  _trees[i] = TreeType(arena,
			       _max_depth,
			       _min_samples,
			       _weight_decay,
			       _reg_lambda,
			       _gamma);

	
        
	  _trees[i].fit(data, shelves, g_sub_ptr, h_sub_ptr, partition);
	  fm = fm + (_trees[i].predict(X) * _learning_rate);
      
	  if (verbose && i % 5 == 0) {
	    ::std::cout << "[*] Learner #" << i << " loss: "
			<< _loss_profile.loss_func(y, fm) << ::std::endl;
	  }

	  arena_clear(arena);
	}
      }, data);
  });

  a.wait();
}

template <typename TreeType>
CNum::DataStructs::Matrix<double> GBModel<TreeType>::predict(CNum::DataStructs::Matrix<double> &data) {
  auto preds = CNum::DataStructs::Matrix<double>::init_const(data.get_rows(), 1, 0);
  
  ::std::for_each(_trees, _trees + _n_learners, [&] (TreeBooster &t) {
    auto t_preds = t.predict(data);
    preds = preds + (t_preds * _learning_rate);
  });

  if (_activation_func) {
    preds = CNum::Model::Activation::activate(preds, _activation_func);
  }
  
  return preds;
}

// ----------------------------
// Saving and loading models
// ----------------------------

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
    throw ::std::runtime_error("GB model saving error - Error opening file");
  }

  of << json_str;
}

template <typename TreeType>
TreeBoosterNode *GBModel<TreeType>::parse_learner(json node) {
  auto *res = new TreeBoosterNode();
  
  res->_split = Split{ node["split"]["feature"],
		       node["split"]["threshold"],
		       1e-4,
		       0,
		       SplitValuePair{ 0, 0 } };
  res->_value = node["value"];
  res->_left = node["left"].dump() == "{}" ? nullptr : parse_learner(node["left"]);
  res->_right = node["right"].dump() == "{}" ? nullptr : parse_learner(node["right"]);

  return res;
}

template <typename TreeType>
GBModel<TreeType> GBModel<TreeType>::load_model(::std::string path) {
  ::std::ifstream is(path);

  if (!is.is_open()) {
    throw ::std::runtime_error("GB model loading error - file could not be opened");
  }

  json data = json::parse(is);
  GBModel<TreeType> res(data["loss_type"],
			data["n_learners"],
			data["learning_rate"],
			data["subsample"],
			data["max_depth"],
			data["min_samples"],
			HIST, // As of now there is only HIST, but GREEDY will come in next release
			data["activation"],
			data["weight_decay"],
			data["reg_lambda"],
			data["gamma"]);

  auto learners = data["learners"];
  int ctr = 0;
  for (auto &[key, value]: learners.items()) {
    auto *tree = GBModel<TreeType>::parse_learner(value);
    res._trees[ctr].set_root(::std::exchange(tree, nullptr));
    ctr++;
  }
  
  return res;
}
