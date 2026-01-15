#ifndef DEPLOY_H
#define DEPLOY_H

#include "crow.h"
#include <functional>
#include <string.h>

/**
 * @namespace CNum::Deploy
 * @brief Tools for creating REST APIs
 */
namespace CNum::Deploy {
  /// @brief The maximum length of a path for a route in the REST API
  constexpr size_t MAX_URL_LEN = 50;


  /// @brief The path of a route in the REST API
  ///
  /// This struct is necessary because of Crow C++ requirements for strings
  /// when adding a route. Typically a string literal is enough, but I wanted to 
  /// create an abstraction for adding routes to the API so a struct 
  /// that can be cast to a constexpr std::string and constexpr 
  /// crow::black_magic::const_str is needed
  struct PathString {
    char str[MAX_URL_LEN];

    constexpr PathString(const char (&s)[MAX_URL_LEN]) {
      for (size_t i = 0; i < MAX_URL_LEN; i++) {
	str[i] = s[i];
      }
    }

    constexpr operator crow::black_magic::const_str() const {
      return crow::black_magic::const_str(str);
    }

    constexpr operator ::std::string() const {
      return ::std::string(str);
    }
  };
  
  /**
   * @class InferenceAPI
   * @brief A REST API for making predictions with CNum models
   * @tparam ModelType The type of CNum model to use for the model pool
   * @tparam Storage A struct containing all of the data from the 
   * preprocessing that also needs to be used in the postprocessing
   */
  template <typename ModelType, typename Storage>
  class InferenceAPI {
    using PreprocessFunctionType = CNum::DataStructs::Matrix<double>(crow::json::rvalue &,
								     crow::json::wvalue &,
								     Storage &);
    using PreprocessFunction = ::std::function< PreprocessFunctionType >;
    
    using PostprocessFunctionType = void(CNum::DataStructs::Matrix<double> &,
					 crow::json::wvalue &,
					 Storage &);
    using PostprocessFunction = ::std::function< PostprocessFunctionType >;
    
    using InferenceRouteFunctionType = void(const crow::request &, crow::response &, ModelType *);
    using InferenceRouteFunction = ::std::function< InferenceRouteFunctionType >;
    using RegularRouteFunctionType = void(const crow::request &, crow::response &);
    using RegularRouteFunction = ::std::function< InferenceRouteFunctionType >;
    
  private:
    ::CNum::Model::ModelPool<ModelType> _models;
    crow::App<crow::CORSHandler> _app;
    PreprocessFunction _preprocess;
    PostprocessFunction _postprocess;
    unsigned short _port;
    
  public:
    /// @brief Constructor
    /// @param path The path to the trained CNum model (".cmod")
    /// @param preprocess The function used to preprocess the data received in the
    /// request to the '/predict' route
    /// @param postprocess The function used process the predictions of the model
    /// into what is returned in the response
    /// @param allowed_origins The allowed origins for the API CORS header
    /// @param n_models The number of model instances in the ModelPool
    /// @param port The port to which the API listens
    InferenceAPI(::std::string path,
		 PreprocessFunction preprocess,
		 PostprocessFunction postprocess,
		 ::std::string allowed_origins = "*",
		 size_t n_models = 20,
		 unsigned short port = 18080);

    /// @brief Add a route to the API that uses a model
    /// @tparam Path The path for the new route
    /// @param method The HTTP method of the route (See Crow C++ documentation)
    /// @param route The actual middleware
    template <PathString Path>
    constexpr void add_inference_route(crow::HTTPMethod method, InferenceRouteFunction route);

    /// @brief Add a route that doesn't use a model
    /// @tparam Path The path for the new route
    /// @param method The HTTP method of the route (See Crow C++ documentation)
    /// @param route The actual middleware
    template <PathString Path>
    constexpr void add_regular_route(crow::HTTPMethod method, RegularRouteFunction route);

    /// @brief Start the backend
    void start();
  };

  #include "CNum/Deploy/Deploy.tpp"
};

#endif
