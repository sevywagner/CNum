template <typename T>
::CNum::DataStructs::Matrix<T> BinaryMask::mask(const CNum::DataStructs::Matrix<T> &m) const {
  if (_size != m.get_rows())
    throw ::std::invalid_argument("BinaryMask mask error - Matrix argument rows not equal to size of the mask");

  size_t n_cols = m.get_cols();
  size_t res_it{ 0 };
  auto res = ::std::make_unique<T[]>(_n_set * n_cols);
    
  uint8_t offset_ctr{ 0 };
  size_t curr{ 0 };
  for (size_t i = 0; i < m.get_rows(); i++) {
    if (_bit_mask[curr] & (1 << offset_ctr++)) {
      auto row_view = m.get_row_view(i);
      ::std::copy(row_view.begin(), row_view.end(), res.get() + res_it);
      res_it += n_cols;
    }

    if (offset_ctr >= 8) {
      curr++;
      offset_ctr = 0;
    }
  }

  return CNum::DataStructs::Matrix<T>(_n_set, n_cols, ::std::move(res));
}

template <typename T, typename CompareFunction>
BinaryMask BinaryMask::create_binary_mask_matrix(const ::CNum::DataStructs::Matrix<T> &m, T val) {
  if (m.get_cols() > 1)
    throw ::std::invalid_argument("BinaryMask creation error - Matrix arg must have only 1 col");
  
  return create_binary_mask< ::CNum::DataStructs::Matrix<T>, T, CompareFunction >(m, val);
}

template <typename T, typename U, typename CompareFunction>
BinaryMask BinaryMask::create_binary_mask(const T &m, U val) {
  CompareFunction comp{};
  size_t size_bytes = n_bytes_required(m.size());
  BitMask mask = ::std::make_unique<uint8_t[]>(size_bytes);

  uint8_t offset_ctr{ 0 };
  size_t curr{ 0 };
  size_t n_set{ 0 };
  for (const auto &el: m) {
    if (comp(el, val)) {
      mask[curr] |= (1 << offset_ctr);
      n_set++;
    }

    if (++offset_ctr >= 8) {
      curr++;
      offset_ctr = 0;
    }
  }

  return BinaryMask(::std::move(mask), m.size(), n_set);
}
