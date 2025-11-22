template <typename ModelType>
ModelPool<ModelType>::ModelPool(::std::string path, size_t n_models) : _n_models(n_models) {
  auto loaded_model = ModelType::load_model(path);
  _model_ptrs.reserve(n_models);
  
  for (size_t i = 0; i < _n_models; i++) {
    ModelType *model = new ModelType();
    *model = loaded_model;
    _model_ptrs.push_back(model);
    _models.enqueue(model);
  }
}

template <typename ModelType>
ModelPool<ModelType>::~ModelPool() {
  for (auto ptr: _model_ptrs)
    delete ptr;
}

template <typename ModelType>
ModelType *ModelPool<ModelType>::pull() {
  ModelType *ptr;
  _models.dequeue(ptr);
  return ptr;
}

template <typename ModelType>
void ModelPool<ModelType>::push(ModelType *model) { _models.enqueue(model); }
