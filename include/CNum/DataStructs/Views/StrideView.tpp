// ---- Overloaded Constructor ----
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

// ---- Comparison BIN mask creation ----
template <typename T>
CNum::DataStructs::Mask<BIN, bool> StrideView<T>::comparison_mask(T comp_val, std::function< bool(T, T) > comp) {
  auto mask_ptr = std::make_unique<bool[]>(_range);
  auto it = _begin;
  int pos_ctr{ 0 };
  
  for (int i = 0; i < _range; i++) {
    mask_ptr[i] = comp(*it, comp_val);
    ++it;
    
    if (mask_ptr[i])
      pos_ctr++;
  }

  return Mask<BIN, bool>(_range, pos_ctr, std::move(mask_ptr));
}

template <typename T>
CNum::DataStructs::Mask<BIN, bool> StrideView<T>::operator<=(T val) {
  return comparison_mask(val, [] (T a, T b) { return a <= b; });
}
