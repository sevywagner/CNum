template <typename ModelType, typename Storage>
InferenceAPI<ModelType, Storage>::InferenceAPI(::std::string path,
				      PreprocessFunction preprocess,
				      PostprocessFunction postprocess,
				      size_t n_models)
  : _models(path, n_models),
    _preprocess(preprocess),
    _postprocess(postprocess) {
  auto &cors = _app.get_middleware<crow::CORSHandler>();
  cors
    .global()
    .origin("*")
    .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post, crow::HTTPMethod::Options)
    .headers("Content-Type");
  
  CROW_ROUTE(_app, "/predict").methods(crow::HTTPMethod::Post)([this] (const crow::request &req) {
    auto req_body = crow::json::load(req.body);
    if (!req_body) return crow::response(400);

    crow::json::wvalue res_body;
    Storage s;
    
    auto inference_data = _preprocess(::std::ref(req_body), ::std::ref(res_body), ::std::ref(s));
    
    auto *model = _models.pull();
    if (!model) {
      return crow::response(500);
    }
    _models.push(model);
    
    auto preds = model->predict(inference_data);
    _models.push(model);

    _postprocess(::std::ref(preds), ::std::ref(res_body), ::std::ref(s));

    crow::response res(201);
    res.body = res_body.dump();
    return res;
  });
}


template <typename ModelType, typename Storage>
template <PathString Path> constexpr void InferenceAPI<ModelType, Storage>::add_inference_route(crow::HTTPMethod method,
												 InferenceRouteFunction route) {
  constexpr auto p = static_cast<::crow::black_magic::const_str>(Path);
  _app.route<crow::black_magic::get_parameter_tag(p)>(Path.str).methods(method)([this, &route] (const crow::request &req,
												crow::response &res) {
    auto *model = _models.pull();
    route(req, res, model);
    _models.push(model);
  });
}


template <typename ModelType, typename Storage>
template <PathString Path> constexpr void InferenceAPI<ModelType, Storage>::add_regular_route(crow::HTTPMethod method,
											       RegularRouteFunction route) {
  constexpr auto p = static_cast<::crow::black_magic::const_str>(Path);
  _app.route<crow::black_magic::get_parameter_tag(p)>(Path.str).methods(method)(route);
}


template <typename ModelType, typename Storage>
void InferenceAPI<ModelType, Storage>::start() {
  _app.port(18080).multithreaded().run();
}
