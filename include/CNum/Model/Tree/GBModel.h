#ifndef GB_REGRESSOR_H
#define GB_REGRESSOR_H

#include "CNum/Utils/Utils.h"
#include "CNum/Data/Data.h"
#include "CNum/Model/Model.h"
#include "CNum/Model/Tree/TreeDefs.h"
#include "json.hpp"
#include <variant>

namespace CNum::Model::Tree {
  struct Split;
  using json = ::nlohmann::json;
  using SubsampleFunction = ::std::function< void(size_t *, size_t, size_t, size_t, ::CNum::DataStructs::Matrix<double>) >;

  inline SubsampleFunction default_subsample = [] (size_t *pos_ptr,
						   size_t low,
						   size_t high,
						   size_t n_samples,
						   const ::CNum::DataStructs::Matrix<double> y) -> void {
    if (low == 0 && high == n_samples) {
      ::std::iota(pos_ptr, pos_ptr + n_samples, low);
    } else {
      ::CNum::Utils::Rand::generate_n_unique_rand_in_range<size_t>(low,
								 high - 1,
								 pos_ptr,
								 n_samples);
    }
  };

  /**
   * @brief The algorithm used for tree finding splits in tree building
   *
   * GREEDY is the exact greedy method (available in 0.3.0)
   * HIST is the histogram method
   */
  enum SplitAlg {
    GREEDY,
    HIST
  };

  struct TreeBoosterNode;

  /**
   * @class GBModel
   * @brief A gradient-boosting model for any child of the TreeBooster class
   * @tparam TreeType A child of the TreeBooster class
   *
   * The GBModel is a robust gradient-boosting model that can be used for both
   * regression and classification on tabular data; simply set the loss and 
   * activation parameters of the constructor accordingly (see Loss and 
   * Activation classes) for available Loss and Activation functions.
   */
  template <typename TreeType>
  class GBModel {
  private:
    TreeType *_trees;
    ::std::string _loss_type;
    CNum::Model::Loss::LossProfile _loss_profile;
    CNum::Model::Activation::ActivationFunc _activation_func;
    double _learning_rate;
    size_t _n_learners;
    double _subsample;
    int _max_depth;
    int _min_samples;
    double _weight_decay;
    ::std::string _activation;
    double _reg_lambda;
    double _gamma;
    SplitAlg _sa;
    SubsampleFunction _subsample_function;

    /// @brief Parse the JSON data for a singular learner and create the TreeBooster
    /// object for it
    /// @param node The JSON data
    /// @return A raw pointer to the root of the TreeBooster
    static TreeBoosterNode *parse_learner(json node);

    /// @brief Used to copy the hyperparameters from one GBModel to another
    void copy_hyperparams(const GBModel &other) noexcept;

    /// @brief The copy logic
    void copy(const GBModel &other) noexcept;

    /// @brief The move logic
    void move(GBModel &&other) noexcept;

  public:
    /**
     * @brief Overloaded default constructor
     * @param lt Loss Type (the name of the loss function)
     * @param n_learners The number of TreeBooster learners to use
     * @param lr The learning rate
     * @param ss The amount of the dataset to sample for each tree
     * @param md The max depth of each tree
     * @param ms The minimum number of samples required to continue building a tree
     * @param sa Split Algorithm; The method used to find the best splits at each
     * node (GREEDY or HIST)
     * @param activation_func The name of the activation function to apply after
     * making predictions
     * @param weight_decay A threshold used to detirmine whether a split is worth
     * considering
     * @param rl Reg Lambda; A regularization parameter used in gain calculations
     * @param gamma A regularization parameter used in gain calculations
     * @param ssf The function used to take subsamples of the data that a TreeBooster
     * is trained on
     *
     * 
     * In the tree building process a split is only taken if the sum of the hessians
     * in the resultant data partitions are both greater than weight_decay.
     *
     * See TreeBoosterNode to understand how the regularization parameters are used
     */
    GBModel(std::string lt = "MSE",
	    int n_learners = 200,
	    double lr = 0.1,
	    double ss = 0.25,
	    int md = 5,
	    int ms = 3,
	    SplitAlg sa = HIST,
	    ::std::string activation_func = "",
	    double weight_decay = 0.0,
	    double rl = 1.0,
	    double gamma = 0.0,
	    SubsampleFunction ssf = default_subsample);

    /**
     * @brief Parameterized constructor
     * @param loss_profile The LossProfile for the loss function
     * @param n_learners The number of TreeBooster learners to use
     * @param lr The learning rate
     * @param ss The amount of the dataset to sample for each tree
     * @param md The max depth of each tree
     * @param ms The minimum number of samples required to continue building a tree
     * @param sa Split Algorithm; The method used to find the best splits at each
     * node (GREEDY or HIST)
     * @param activation_func The activation function to apply after making predictions
     * @param weight_decay A threshold used to detirmine whether a split is worth
     * considering
     * @param rl Reg Lambda; A regularization parameter used in gain calculations
     * @param gamma A regularization parameter used in gain calculations
     * @param ssf The function used to take subsamples of the data that a TreeBooster
     * is trained on
     *
     * 
     * In the tree building process a split is only taken if the sum of the hessians
     * in the resultant data partitions are both greater than weight_decay.
     *
     * See TreeBoosterNode to understand how the regularization parameters are used
     */
    GBModel(::CNum::Model::Loss::LossProfile loss_profile,
	    int n_learners,
	    double lr,
	    double ss,
	    int md,
	    int ms,
	    SplitAlg sa,
	    ::CNum::Model::Activation::ActivationFunc activation_func,
	    double weight_decay,
	    double rl,
	    double gamma,
	    SubsampleFunction ssf = default_subsample);


    /// @brief Copy constructor
    GBModel(const GBModel &other) noexcept;

    /// @brief Copy assignment
    GBModel<TreeType> &operator=(const GBModel &other) noexcept;

    /// @brief Move constructor
    GBModel(GBModel &&other) noexcept;
    
    /// @brief Move assignment
    GBModel<TreeType> &operator=(GBModel &&other) noexcept;

    /// @brief Destructor
    ~GBModel();

    /// @brief Train the model
    /// @param X The tabular data used to train the GBModel
    /// @param y The labels for the data (the intended output of the model)
    void fit(::CNum::DataStructs::Matrix<double> &X,
		     ::CNum::DataStructs::Matrix<double> &y);

    /// @brief Inference (making predictions)
    /// @param The data to make predictions on
    /// @return The predictions
    ::CNum::DataStructs::Matrix<double> predict(::CNum::DataStructs::Matrix<double> &data);

    /// @brief Save Model to JSON encoded ".cmod" file
    /// @param path The path to save the file to
    void save_model(std::string path);

    /// @brief Load Model from JSON encoded ".cmod" file
    /// @param path The path to the ".cmod" file
    /// @return The GBModel
    static GBModel<TreeType> load_model(std::string path);
  };

#include "GBModel.tpp"
};

#endif
