#ifndef CONCURRENT_QUEUE
#define CONCURRENT_QUEUE

#include <mutex>
#include <queue>
#include <condition_variable>

namespace CNum::DataStructs {

  /**
   * @class ConcurrentQueue
   * @brief Coarse-grained locking concurrent queue
   *
   * This queue was designed to be used for the thread pool. Perf tests confirmed 
   * the course-grained locking approach had perf benefits under load compared to
   * a lock-free approach. This is likely due to the way the pool is used in
   * the CNum ML models which is submitting many instances of smaller tasks. 
   * If you need a thread pool with a lock free queue this queue and the
   * ThreadPool class can easily be swapped with the lock-free version on my
   * GitHub
   */
  template <typename T>
  class ConcurrentQueue {
  private:
    std::mutex _mtx;
    std::queue<T> _q;
    std::condition_variable _cv;
    bool _stop;
  
  public:
    /// @brief Default constructor
    ConcurrentQueue();
    ~ConcurrentQueue() = default;

    /// @brief Enqueue a element
    /// @param val The element
    void enqueue(T val);

    /// @brief dequeue a value
    /// @param val Reference to the variable to store the popped value in
    /// @return Whether or not the operation was successful
    bool dequeue(T &val);

    /// @brief Set stop flag for thread pool
    /// 
    /// This is used to set the stop flag which tells the thread pool
    /// not to enqueue any more tasks
    void set_stop();

    /// @brief Check if the queue is empty
    /// @return Whether or not the queue is empty
    bool empty();

    /// @brief Check stop flag for thread pool
    /// @return Whether or not the stop flag is set
    bool stop_is_set();
  };

#include "CNum/DataStructs/ConcurrentQueue.tpp"
};

#endif
