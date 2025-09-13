#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <CNum.h>
#include <memory>
#include <atomic>
#include <vector>
#include <future>

using namespace CNum::DataStructs;
using namespace CNum::Model::Tree;

TEST_CASE("Arena") {
  arena_t *arena = arena_init(10);
  for (int i = 0; i < 20; i++) {
    arena_view_t view = arena_malloc(arena, sizeof(double), sizeof(double));
  }

  arena_free(arena);
  // Leak Sanitizer and Address Sanitizer will determine the passing of this test
}

TEST_CASE("Thread Pool") {
  ::std::atomic<int> ctr{ 0 };
  std::function< void(arena_t *) > task = [&ctr] (arena_t *arena) { ctr++; };

  int n_threads = 1000;
  ::std::vector< ::std::future<void> > futures;
  futures.reserve(n_threads);
  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();
  for (int i = 0; i < n_threads; i++)
    futures.push_back(tp->submit< void >(task));

  for (auto &f: futures)
    f.wait();

  CHECK(ctr.load() == n_threads);
}

TEST_CASE("Training, saving, and loading models") {
  constexpr size_t len = 100;
  
  auto x = ::std::make_unique<double[]>(len);
  auto y = ::std::make_unique<double[]>(len);
  ::std::iota(x.get(), x.get() + len, 0.0);
  ::std::iota(y.get(), y.get() + len, 0.0);

  Matrix<double> x_matrix(len, 1, ::std::move(x));
  Matrix<double> y_matrix(len, 1, ::std::move(y));

  GBModel<XGTreeBooster> xgboost("MSE", 10 /* n_learners */);
  xgboost.fit(x_matrix, y_matrix);
  xgboost.save_model("./tests/model.cmod");

  auto xgboost_loaded = GBModel<XGTreeBooster>::load_model("./tests/model.cmod");
  auto trained_preds = xgboost.predict(x_matrix);
  auto loaded_preds = xgboost_loaded.predict(x_matrix);

  for (int i = 0; i < trained_preds.get_rows(); i++) {
    CHECK(abs(trained_preds.get(i, 0) - loaded_preds.get(i, 0)) < 1e-4);
  }
}
