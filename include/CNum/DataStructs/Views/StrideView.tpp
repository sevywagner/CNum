template <typename T>
StrideView<T>::StrideView(T *ptr, size_t stride, size_t range)
  : _ptr(ptr), _stride(stride), _range(range) {
  _begin = StrideIterator(ptr, stride);
  _end = StrideIterator(ptr + (stride * range), stride);
}


template <typename T>
StrideIterator<T> StrideView<T>::begin() const { return _begin; }


template <typename T>
StrideIterator<T> StrideView<T>::end() const { return _end; }


template <typename T>
BinaryMask StrideView<T>::operator<=(T val) {
  return CNum::DataStructs::BinaryMask::create_binary_mask< StrideView<T>, T, ::std::less_equal<T> >(*this, val);
}

template <typename T>
size_t StrideView<T>::size() const { return _range; }
