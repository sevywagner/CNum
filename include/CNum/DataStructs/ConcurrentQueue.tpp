template <typename T>
ConcurrentQueue<T>::ConcurrentQueue() : _stop(false) {}

template <typename T>
void ConcurrentQueue<T>::enqueue(T val) {
  ::std::unique_lock<std::mutex> ul(_mtx);

  _q.push(val);
  ul.unlock();
  _cv.notify_one();
}

template <typename T>
bool ConcurrentQueue<T>::dequeue(T &val) {
  ::std::unique_lock<std::mutex> ul(_mtx);

  _cv.wait(ul, [this] { return !_q.empty() || _stop; });

  if (_q.empty() && _stop) {
    return false;
  }
  
  val = _q.front();
  _q.pop();
  return true;
}

// ---- Set stop flag for thread pool ----
template <typename T>
void ConcurrentQueue<T>::set_stop() {
  std::lock_guard<std::mutex> lg(_mtx);
  _stop = true;
  _cv.notify_all();
}

// ---- Check stop flag for thread pool ----
template <typename T>
bool ConcurrentQueue<T>::stop_is_set() {
  std::lock_guard<std::mutex> lg(_mtx);
  return _stop;
}

template <typename T>
bool ConcurrentQueue<T>::empty() {
  std::lock_guard<std::mutex> lg(_mtx);
  return _q.empty();
}
