template <typename Container, typename T, typename CompareFunction>
IndexMask IndexMask::argsort(const Container &container) {
  CompareFunction comp{};
  auto idx_ptr = ::std::make_unique<size_t[]>(container.size());
  ::std::iota(idx_ptr.get(), idx_ptr.get() + container.size(), 0);
  ::std::sort(idx_ptr.get(), idx_ptr.get() + container.size(), [&container, &comp] (const size_t &a, const size_t &b) {
    return comp(container[a], container[b]);
  });
  
  return IndexMask(::std::move(idx_ptr), container.size());
}

template <typename T>
::CNum::DataStructs::Matrix<T> IndexMask::matrix_apply_mask(const ::CNum::DataStructs::Matrix<T> &m) const {
  size_t n_cols = m.get_cols();
  auto res_ptr = ::std::make_unique<T[]>(_size * n_cols);
  size_t res_it{ 0 };
    
  for (size_t i = 0; i < _size; i++) {
    auto row_view = m.get_row_view(_mask[i]);
    ::std::copy(row_view.begin(), row_view.end(), res_ptr.get() + res_it);
    res_it += n_cols;
  }

  return ::CNum::DataStructs::Matrix<T>(_size, n_cols, ::std::move(res_ptr));
}

template <typename T>
::CNum::DataStructs::Matrix<T> IndexMask::matrix_apply_mask_col_wise(const ::CNum::DataStructs::Matrix<T> &m) const {
  auto res_ptr = ::std::make_unique<T[]>(m.get_rows() * _size);
  for (size_t i = 0; i < _size; i++) {
    auto col_view = m.get_col_view(_mask[i]);
    auto col_it = col_view.begin();
    for (size_t j = 0; j < m.get_rows(); j++) {
      res_ptr[j * _size + i] = *col_it;
      ++col_it;
    } 
  }

  return ::CNum::DataStructs::Matrix<T>(m.get_rows(), _size, ::std::move(res_ptr));
}
