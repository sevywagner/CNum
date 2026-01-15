#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "CNum/DataStructs/ConcurrentQueue.h"
#include "CNum/DataStructs/Memory/Arena.h"

#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <atomic>

/**
 * @namespace CNum::Multithreading
 * @brief Structures and algorithms used for multithreaded operations
 */
namespace CNum::Multithreading {
  using Func = std::function< void(arena_t *) >;

  /// @brief The default amount of blocks allocated to a newly initialized arena
  constexpr size_t default_arena_init_block_ct = 16500;

  /// @brief The configuration of the ThreadPool
  struct ThreadPoolConfig {
    size_t arena_blocks_to_allocate;
  };

  /**
   * @class ThreadPool
   * @brief A promise based thread pool
   */
  class ThreadPool {
  private:
    ::CNum::DataStructs::ConcurrentQueue< Func > _q;
    int _num_threads;
    std::vector<std::thread> _threads;
    std::mutex _mtx;
    ::std::atomic<bool> _is_shutdown;
    static thread_local int _worker_id;

    /// @brief The worker used for each thread in the pool
    /// @param The configuration of the ThreadPool
    /// @param worker_id The index of the worker thread in the pool
    void worker(ThreadPoolConfig config, int worker_id);

    /// @brief Overloaded constructor
    ThreadPool(ThreadPoolConfig config);
  

  public:
    /// @brief Get the instance of the ThreadPool singleton
    /// @param config The configuration of the ThreadPool
    /// @return A raw pointer to the instance of the ThreadPool
    static ThreadPool *get_thread_pool(ThreadPoolConfig config = { default_arena_init_block_ct });

    /// @brief Get the thread-local id of a worker
    /// @return The worker id
    static int get_worker_id();

    /// @brief Destructor
    ~ThreadPool();
  
    ThreadPool(const ThreadPool &other) = delete;
    ThreadPool &operator=(const ThreadPool &other) = delete;
    ThreadPool(ThreadPool &&other) = delete;
    ThreadPool &operator=(ThreadPool &&other) = delete;

    /// @brief Shut the ThreadPool down
    void shutdown();
    
    /// @brief Submit a task to the ThreadPool
    /// @param f The task
    /// @return A future that will contain whatever the task returns
    template<typename T>
    std::future<T> submit(std::function< T(arena_t *arena) > f) {
      if (_is_shutdown.load(::std::memory_order_acquire))
	throw ::std::runtime_error("Thread pool submit error - Submit cannot be called after thread pool is shut down");
      
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
