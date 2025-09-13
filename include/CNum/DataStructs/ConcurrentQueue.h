#ifndef __CONCURRENT_QUEUE
#define __CONCURRENT_QUEUE

#include <mutex>
#include <queue>
#include <condition_variable>

namespace CNum::DataStructs {
  template <typename T>
  class ConcurrentQueue {
  private:
    std::mutex _mtx;
    std::queue<T> _q;
    std::condition_variable _cv;
    bool _stop;
  
  public:
    ConcurrentQueue();
    ~ConcurrentQueue() = default;

    void enqueue(T val);
    bool dequeue(T &val);

    void set_stop();
    
    bool empty();
    bool stop_is_set();
  };

#include "CNum/DataStructs/ConcurrentQueue.tpp"
};

#endif
