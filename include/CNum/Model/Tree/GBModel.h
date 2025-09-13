#ifndef __GB_REGRESSOR_H
#define __GB_REGRESSOR_H

#include "json.hpp"
#include <variant>

namespace CNum::Model::Tree {
  using json = ::nlohmann::json;

  enum split_alg {
    GREEDY,
    HIST
  };

  template <typename TreeType>
  class GBModel {
  private:
    TreeBooster *_trees;
    std::string _loss_type;
  
    double _learning_rate;
    size_t _n_learners;
    double _subsample;
    int _max_depth;
    int _min_samples;
    double _weight_decay;
    ::std::string _activation;
    double _reg_lambda;
    double _gamma;
    enum split_alg _sa;

    static TreeBoosterNode *parse_learner(json node);

  public:
    GBModel(std::string lt = "MSE",
	    int n_learners = 200,
	    double lr = 0.1,
	    double ss = 0.25,
	    int md = 5,
	    int ms = 3,
	    enum split_alg sa = HIST,
	    ::std::string activation_func = "",
	    double weight_decay = 0.0,
	    double rl = 1.0,
	    double gamma = 0.0);
  
    ~GBModel();

    void fit(CNum::DataStructs::Matrix<double> &X,
	     CNum::DataStructs::Matrix<double> &y);
    CNum::DataStructs::Matrix<double> predict(CNum::DataStructs::Matrix<double> &data);

    void save_model(std::string path);
    static GBModel<TreeType> load_model(std::string path);
  };

#include "GBModel.tpp"
};

#endif
