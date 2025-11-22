#ifndef MODEL_POOL_H
#define MODEL_POOL_H

#include <memory>
#include <vector>

namespace CNum::Model {
  /**
   * @class ModelPool
   * @brief A pool of trained models
   */
  template <typename ModelType>
  class ModelPool {
  private:
    size_t _n_models;
    ::CNum::DataStructs::ConcurrentQueue<ModelType *> _models;
    ::std::vector<ModelType *> _model_ptrs;
    
  public:
    /// @brief Overloaded default constructor
    /// @param path Path to your trained model (.cmod)
    /// @param n_models The number of models in the pool
    ModelPool(::std::string path, size_t n_models = 20);

    ModelPool(const ModelPool &) = delete;
    ModelPool &operator=(const ModelPool &) = delete;

    /// @brief Destructor
    ~ModelPool();

    /// @brief Pull a model from the pool
    /// @return A raw pointer to a trained model
    ModelType *pull();

    /// @brief Push a model to the pool
    /// model A raw pointer to a trained model pulled from the pool
    void push(ModelType *model);
  };

  #include "CNum/Model/ModelPool.tpp"
};

#endif
