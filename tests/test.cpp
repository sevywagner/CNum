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
  xgboost.save_model("model.cmod");

  auto xgboost_loaded = GBModel<XGTreeBooster>::load_model("model.cmod");
  auto trained_preds = xgboost.predict(x_matrix);
  auto loaded_preds = xgboost_loaded.predict(x_matrix);

  for (int i = 0; i < trained_preds.get_rows(); i++) {
    CHECK(abs(trained_preds.get(i, 0) - loaded_preds.get(i, 0)) < 1e-4);
  }
}

TEST_CASE("Binary mask") {
  constexpr size_t len = 10;

  auto ptr2 = ::std::make_unique<double[]>(len * len);
  ::std::iota(ptr2.get(), ptr2.get() + (len * len), 1);
  
  auto ptr = ::std::make_unique<double[]>(len);
  ptr[0] = 1.1;
  ptr[1] = 4.3;
  ptr[2] = 2.5;
  ptr[3] = 1.6;
  ptr[4] = 3.7;
  ptr[5] = 3.1;
  ptr[6] = 4.5;
  ptr[7] = 1.9;
  ptr[8] = 5.0;
  ptr[9] = 4.1;

  Matrix<double> m(len, 1, ::std::move(ptr));
  Matrix<double> m3(len, len, ::std::move(ptr2));
  
  auto mask = m < 2.5;
  auto m2 = m[mask];
  auto m4 = m3[mask];
  
  m.print_matrix();

  ::std::cout << "---------" << ::std::endl;
  
  m2.print_matrix();

  ::std::cout << "---------" << ::std::endl;

  m4.print_matrix();

  ::std::cout << "---------" << ::std::endl;

  auto col_view = m3.get_col_view(3);
  auto mask2 = col_view <= 50;
}

TEST_CASE("Index mask") {
  constexpr size_t len = 10;

  auto ptr = ::std::make_unique<double[]>(len * len);
  ::std::iota(ptr.get(), ptr.get() + (len * len), 1);
  
  auto ptr2 = ::std::make_unique<double[]>(len);
  ptr2[0] = 1.1;
  ptr2[1] = 4.3;
  ptr2[2] = 2.5;
  ptr2[3] = 1.6;
  ptr2[4] = 3.7;
  ptr2[5] = 3.1;
  ptr2[6] = 4.5;
  ptr2[7] = 1.9;
  ptr2[8] = 5.0;
  ptr2[9] = 4.1;

  Matrix<double> m(len, len, ::std::move(ptr));
  Matrix<double> m2(len, 1, ::std::move(ptr2));

  auto mask = m2.argsort();
  auto m3 = m2[mask];
  m3.print_matrix();

  auto res = m.col_wise_mask_application(mask);
  res.print_matrix();
}
