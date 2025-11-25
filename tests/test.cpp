#include <gtest/gtest.h>
#include <CNum.h>
#include <memory>
#include <atomic>
#include <vector>
#include <future>
#include <thread>
#include <chrono>

using namespace ::std::chrono_literals;

using namespace CNum::DataStructs;
using namespace CNum::Model::Tree;

// -----------------------
// Read-only global vars
// -----------------------

constexpr size_t gb_suite_len = 100;
Matrix<double> gb_suite_x;
Matrix<double> gb_suite_y;

constexpr size_t mask_suite_len = 10;
Matrix<double> mask_suite_1d;
Matrix<double> mask_suite_2d;

// --------
// Tests
// --------

TEST(GBModelSuite, TestTrain) {  
  constexpr double tolerance = 1;

  GBModel<XGTreeBooster> xgboost("RMSE", 300 /* n_learners */, 0.25 /* subsample */);
  xgboost.fit(gb_suite_x, gb_suite_y, false);

  auto preds = xgboost.predict(gb_suite_x);
  for (int i = 0; i < preds.size(); i++) {
	ASSERT_TRUE(abs(gb_suite_y[i] - preds[i]) < tolerance);
  }
}

TEST(GBModelSuite, TestTrainPredictSaveLoad) {
  constexpr double tolerance = 1e-4;
  GBModel<XGTreeBooster> xgboost("MSE", 10 /* n_learners */);
  xgboost.fit(gb_suite_x, gb_suite_y, false);
  xgboost.save_model("model.cmod");

  auto xgboost_loaded = GBModel<XGTreeBooster>::load_model("model.cmod");
  auto trained_preds = xgboost.predict(gb_suite_x);
  auto loaded_preds = xgboost_loaded.predict(gb_suite_x);

  for (int i = 0; i < trained_preds.get_rows(); i++) {
    ASSERT_TRUE(abs(trained_preds.get(i, 0) - loaded_preds.get(i, 0)) < tolerance);
  }
}

TEST(GBModelSuite, DeterminismTest) {
  constexpr double tolerance = 1e-3;

  CNum::Utils::Rand::RandomGenerator::set_global_seed(42);
  ::std::array< Matrix<double>, 2 > preds;

  for (int i = 0; i < 2; i++) {
    CNum::Utils::Rand::RandomGenerator::reset_state();
    GBModel<XGTreeBooster> xgboost("MSE",
				   30 /* n_learners */,
				   .1 /* learning rate */,
				   .15 /* subsample */);

    xgboost.fit(gb_suite_x, gb_suite_y, false);
    preds[i] = xgboost.predict(gb_suite_x);
  }

  for (int i = 0; i < preds[0].size(); i++) {
    ASSERT_TRUE(abs(preds[0][i] - preds[1][i]) < tolerance);
  }
}

TEST(BinaryMask, AllNegativeTest) {
  auto mask = mask_suite_1d == 0.0001;
  auto m2 = mask_suite_1d[mask];
  auto m3 = mask_suite_2d[mask];
  ASSERT_EQ(m2.size(), 0);
  ASSERT_EQ(m3.size(), 0);
}

TEST(BinaryMask, SinglePositiveTest) {
  auto mask = mask_suite_1d == 1.6;
  auto m2 = mask_suite_1d[mask];
  auto m3 = mask_suite_2d[mask];
  
  ASSERT_EQ(m2.size(), 1);
  ASSERT_EQ(m2[0], 1.6);
  
  ASSERT_EQ(m3.get_rows(), 1);
  for (int i = 0; i < mask_suite_len; i++) {
    ASSERT_EQ(m3[i], i + 31); // it is in the 3rd row so we add 31
  }
}

TEST(BinaryMask, AllPositiveTest) {
  auto mask = mask_suite_1d > 0;
  auto m2 = mask_suite_1d[mask];
  auto m3 = mask_suite_2d[mask];
  
  ASSERT_EQ(m2.size(), mask_suite_len);
  for (size_t i = 0; i < mask_suite_len; i++) {
    ASSERT_EQ(m2[i], mask_suite_1d[i]);
  }

  for (int i = 0; i < mask_suite_len; i++) {
    for (int j = 0; j < mask_suite_len; j++) {
      ASSERT_EQ(m3.get(i, j), i * mask_suite_len + j + 1);
    }
  }
}

TEST(IndexMask, ArgsortTest) {
  auto ptr2 = ::std::make_unique<double[]>(mask_suite_len);
  ptr2[0] = 5.0;
  ptr2[1] = 4.5;
  ptr2[2] = 4.3;
  ptr2[3] = 4.1;
  ptr2[4] = 3.7;
  ptr2[5] = 3.1;
  ptr2[6] = 2.5;
  ptr2[7] = 1.9;
  ptr2[8] = 1.6;
  ptr2[9] = 1.1;

  auto mask = mask_suite_1d.argsort();
  auto m2 = mask_suite_1d[mask];

  for (size_t i = 0; i < mask_suite_len; i++) {
    ASSERT_EQ(m2[i], ptr2[i]);
  }
}

TEST(IndexMask, 2dMatrixTest) {
  constexpr size_t idx_mask_len = 3;
  auto ptr2 = ::std::make_unique<size_t[]>(idx_mask_len);
  ptr2[0] = 1;
  ptr2[1] = 7;
  ptr2[2] = 4;

  IndexMask mask(::std::move(ptr2), idx_mask_len);
  auto m2 = mask_suite_2d[mask];

  for (int i = 0; i < mask_suite_len; i++) {
    ASSERT_EQ(m2.get(0, i), i + 11);
  }

  for (int i = 0; i < mask_suite_len; i++) {
    ASSERT_EQ(m2.get(1, i), i + 71);
  }

  for (int i = 0; i < mask_suite_len; i++) {
    ASSERT_EQ(m2.get(2, i), i + 41);
  }
}

TEST(ThreadPoolSuite, SimpleThreadPoolTest) {
  ::std::atomic<int> ctr{ 0 };
  std::function< void(arena_t *) > task = [&ctr] (arena_t *arena) { ctr++; };

  int n_tasks = 1000;
  ::std::vector< ::std::future<void> > futures;
  futures.reserve(n_tasks);
  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();
  for (int i = 0; i < n_tasks; i++)
    futures.push_back(tp->submit< void >(task));

  for (auto &f: futures)
    f.wait();

  ASSERT_EQ(ctr.load(), n_tasks);
}

TEST(ThreadPoolSuite, BurstThreadPoolTest) {
  ::std::atomic<int> ctr{ 0 };
  std::function< void(arena_t *) > task = [&ctr] (arena_t *arena) { ctr++; };

  int n_tasks = 1000;
  ::std::vector< ::std::future<void> > futures;
  futures.reserve(n_tasks);
  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();
  for (int i = 0; i < n_tasks; i++)
    futures.push_back(tp->submit< void >(task));

  ::std::this_thread::sleep_for(1s);
  
  for (int i = 0; i < n_tasks; i++)
    futures.push_back(tp->submit< void >(task));

  ::std::this_thread::sleep_for(1s);
  
  for (int i = 0; i < n_tasks; i++)
    futures.push_back(tp->submit< void >(task));
  
  ::std::this_thread::sleep_for(1s);
  
  for (auto &f: futures)
    f.wait();

  ASSERT_EQ(ctr.load(), n_tasks * 3); // because 3 rounds
}

TEST(ThreadPoolSuite, ThreadPoolShutdownTest) {
  ::std::atomic<int> ctr{ 0 };
  std::function< void(arena_t *) > task = [&ctr] (arena_t *arena) { ctr++; };

  int n_tasks = 1000;
  ::std::vector< ::std::future<void> > futures;
  futures.reserve(n_tasks);
  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();
  for (int i = 0; i < n_tasks; i++)
    futures.push_back(tp->submit< void >(task));

  tp->shutdown();
  bool did_throw{ false };
  try {
    tp->submit< void >(task);
  } catch(...) { did_throw = true; }

  ASSERT_EQ(n_tasks, ctr);
  ASSERT_TRUE(did_throw);
}

int main(int argc, char **argv) {
  // Initialize read-only global vars

  // GB vars
  auto x = ::std::make_unique<double[]>(gb_suite_len);
  auto y = ::std::make_unique<double[]>(gb_suite_len);
  ::std::iota(x.get(), x.get() + gb_suite_len, 0.0);
  ::std::iota(y.get(), y.get() + gb_suite_len, 0.0);

  gb_suite_x = Matrix<double>(gb_suite_len, 1, ::std::move(x));
  gb_suite_y = Matrix<double>(gb_suite_len, 1, ::std::move(y));

  // Mask vars
  auto ptr = ::std::make_unique<double[]>(mask_suite_len);
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

  mask_suite_1d = Matrix<double>(mask_suite_len, 1, ::std::move(ptr));

  size_t size_2d = mask_suite_len * mask_suite_len;
  ptr = ::std::make_unique<double[]>(size_2d);
  ::std::iota(ptr.get(), ptr.get() + size_2d, 1.0);

  mask_suite_2d = Matrix<double>(mask_suite_len, mask_suite_len, ::std::move(ptr));
  
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
