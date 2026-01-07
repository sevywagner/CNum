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

    CNum::DataStructs::Matrix<double> inference_data;
    
     try {
      inference_data = _preprocess(req_body, res_body, s);
    } catch (...) {
      crow::response res(500, "Error occurred in preprocessing function. ");
      return res;
    }
    
    auto *model = _models.pull();
    if (!model) {
      return crow::response(500, "Model Pool Error");
    }
    
    auto preds = model->predict(inference_data);
    _models.push(model);

    try {
      _postprocess(preds, res_body, s);
    } catch (...) {
      crow::response res(500, "Error occurred in postprocessing function. ");
      return res;
    }

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
    if (!model) {
      res = crow::response(500, "Model Pool Error");
      res.end();
      return;
    }

    res = crow::response(200, "Successfully performed task");
    try {
      route(req, res, model);
    } catch (::std::runtime_error e) {
      res = crow::response(500, e.what());
    } catch (...) {
      res = crow::response(500, "Internal Server Error");
    }
    
    _models.push(model);
    res.end();
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
