#ifndef __THREAD_POOL_H
#define __THREAD_POOL_H

#include "CNum/DataStructs/ConcurrentQueue.h"
#include "CNum/Multithreading/Multithreading.h"
#include "CNum/DataStructs/Memory/Arena.h"

#include <functional>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>

namespace CNum::Multithreading {
  using Func = std::function< void(arena_t *) >;
  constexpr size_t default_arena_init_block_ct = 500000;

  struct thread_pool_config {
    size_t arena_blocks_to_allocate;
  };

  class ThreadPool {
  private:
    CNum::DataStructs::ConcurrentQueue< Func > _q;
    int _num_threads;
    std::vector<std::thread> _threads;
    std::mutex _mtx;

    void worker(thread_pool_config config);
    void shutdown();

    ThreadPool(thread_pool_config config);
  

  public:
    static ThreadPool *get_thread_pool(thread_pool_config config = { default_arena_init_block_ct });
    ~ThreadPool();
  
    ThreadPool(const ThreadPool &other) = delete;
    ThreadPool &operator=(const ThreadPool &other) = delete;
    ThreadPool(ThreadPool &&other) = delete;
    ThreadPool &operator=(ThreadPool &&other) = delete;

    template<typename T>
    std::future<T> submit(std::function< T(arena_t *arena) > f) {
      auto p = std::make_shared< std::promise<T> >();
      std::future<T> fut = p->get_future();
  
      auto func = [p, f] (arena_t *arena) mutable {
	if constexpr (std::is_void_v<T>) {
	  f(arena);
	  p->set_value();
	} else {
	  p->set_value(f(arena));
	}
      };
  
      _q.enqueue(func);
      return fut;
    }
  };
};

#endif
