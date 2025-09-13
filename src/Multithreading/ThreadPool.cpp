#include "CNum/Multithreading/ThreadPool.h"

using namespace CNum::Multithreading;

// -----------------------
// Thread Pool Singleton
// -----------------------

namespace CNum::Multithreading {
  ThreadPool::ThreadPool(thread_pool_config config) {
    _num_threads = std::thread::hardware_concurrency() - 1;
    if (_num_threads < 1)
      _num_threads = 1;

    for (int i = 0; i < _num_threads; i++) {
      _threads.emplace_back(&ThreadPool::worker, this, config);
    }
  }

  ThreadPool *ThreadPool::get_thread_pool(thread_pool_config config) {
    static ThreadPool *thread_pool = new ThreadPool(config); // leaks by design to avoid shutdown order issues
    return thread_pool;
  }

  void ThreadPool::shutdown() {
    _q.set_stop();

    for (auto &t: _threads)
      t.join();
  }

  ThreadPool::~ThreadPool() {
    shutdown();
  }

  void ThreadPool::worker(thread_pool_config config) {
    arena_t *arena = arena_init(config.arena_blocks_to_allocate);
    
    Func func;   
    while (_q.dequeue(func)) {
      func(arena);
    }

    arena_free(arena);
  }
};
