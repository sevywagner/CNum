// ---- Overloaded constructor ----
template <typename T>
StrideIterator<T>::StrideIterator(T *ptr, size_t stride)
  : _ptr(ptr), _stride(stride) {}

// ---- Dereference ----
template <typename T>
T StrideIterator<T>::operator*() {
  return *_ptr;
}

// ---- Increment ----
template <typename T>
StrideIterator<T> &StrideIterator<T>::operator++() {
  _ptr += _stride;
  return *this;
}

// ---- Does not equal comparison (by address) ----
template <typename T>
bool StrideIterator<T>::operator!=(const StrideIterator &other) { return this->_ptr != other._ptr; }
