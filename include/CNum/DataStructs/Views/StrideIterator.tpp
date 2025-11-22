template <typename T>
StrideIterator<T>::StrideIterator(T *ptr, size_t stride)
  : _ptr(ptr), _stride(stride) {}


template <typename T>
T StrideIterator<T>::operator*() {
  return *_ptr;
}


template <typename T>
StrideIterator<T> &StrideIterator<T>::operator++() {
  _ptr += _stride;
  return *this;
}


template <typename T>
bool StrideIterator<T>::operator!=(const StrideIterator &other) { return this->_ptr != other._ptr; }
