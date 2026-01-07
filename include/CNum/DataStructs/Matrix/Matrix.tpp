//---------------
// Constructors
//---------------

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t cols, ::std::unique_ptr<T[]> ptr)
  : _cols(cols), _rows(rows), _data(::std::move(ptr))  {
  if (_data == nullptr && _rows > 0 && _cols > 0) {
    _data = ::std::make_unique<T[]>(_rows * _cols);
  }
}

template <typename T>
void Matrix<T>::copy(const Matrix &other) noexcept {
  if (this == &other) return;

  this->_rows = other._rows;
  this->_cols = other._cols;

  if (this->_data != nullptr)
    this->_data.reset();
  
  this->_data = ::std::make_unique<T[]>(other._rows * other._cols);
  ::std::copy(other._data.get(), other._data.get() + other._rows * other._cols, this->_data.get());
}

template <typename T>
Matrix<T>::Matrix(const Matrix &other) noexcept {
  this->copy(other);
}

template <typename T>
Matrix<T> &Matrix<T>::operator=(const Matrix &other) noexcept {
  this->copy(other);
  return *this;
}

template <typename T>
void Matrix<T>::move(Matrix<T> &&other) noexcept {
  if (this == &other) {
    return;
  }

  this->_rows = other._rows;
  other._rows = 0;
  
  this->_cols = other._cols;
  other._cols = 0;

  this->_data = ::std::move(other._data);
}

template <typename T>
Matrix<T>::Matrix(Matrix &&other) noexcept {
  this->move(::std::move(other));
}

template <typename T>
Matrix<T> &Matrix<T>::operator=(Matrix &&other) noexcept {
  this->move(::std::move(other));
  return *this;
}

template <typename T>
Matrix<T>::~Matrix() {}

//-----------------
// Linear Algebra
//-----------------

template <typename T>
Matrix<T> Matrix<T>::element_wise(T val, ::std::function< void(T &, T) > func) const noexcept {
  Matrix<T> res(*this);
  
  ::std::for_each(res._data.get(),
		  res._data.get() + res._rows * res._cols,
		  [&func, &val] (T &t) {
		    func(t, val);
		  });

  return res;
}

template <typename T>
Matrix<T> Matrix<T>::operator*(const Matrix &other) const {
  if (this->_cols != other._rows) {
    throw ::std::invalid_argument("Matrix dot product error - misaligned dims");
  }

  Matrix<T> res(this->_rows, other._cols);
  auto res_ptr = res._data.get();
  
  for (size_t i = 0; i < this->_rows; i++) {
    for (size_t j = 0; j < other._cols; j++) {
      T sum{ 0 };
      
      for (size_t k = 0; k < _cols; k++) {
	sum += this->_data[i * this->_cols + k] * other._data[k * other._cols + j];
      }

      res_ptr[i * res._cols + j] = sum;
    }
  }

  return res;
}

template <typename T>
Matrix<T> Matrix<T>::operator*(T scale_factor) const noexcept {
  return element_wise(scale_factor, [] (T &a, T b) {
    a *= b;
  });
}

template <typename T>
T Matrix<T>::dot(const Matrix<T> &other) const {
  if (this->_rows != other._rows || this->_cols > 1 || other._cols) {
    throw ::std::invalid_argument("Vector dot product error -- Dims misaligned");
  }

  T sum{0};
  for (size_t i = 0; i < _rows; i++) {
    sum += this->_data[i] * other._data[i];
  }

  return sum;
}

// Data must be aligned to the cache-line size to avoid false sharing
template <typename T>
void Matrix<T>::par_execute(uint8_t n_threads,
			    size_t total_el,
			    ::std::function< void(size_t) > callback) {
  
  ::std::vector< ::std::future<void> > workers;
  size_t el_per_thread = total_el / n_threads;

  auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();

  for (uint8_t i = 0; i < n_threads; i++) {
    size_t start = i * el_per_thread;
    size_t end = ::std::min(start + el_per_thread, total_el);

    workers.push_back(tp->submit< void >([start, end, &callback] (arena_t *arena) {
      for (int j = start; j < end; j++) {
	callback(j);
      }
    }));
  };

  for (auto &f: workers)
    f.wait();
}

template <typename T>
Matrix<T> Matrix<T>::operator+(const Matrix &other) const {
  if (this->_rows != other._rows || this->_cols != other._cols) {
    throw ::std::invalid_argument("Matrix addition error - misaligned dims");
  }

  size_t total_el = _rows * _cols;
  auto sum_ptr = ::std::make_unique<T[]>(total_el);

  for (size_t i = 0; i < total_el; i++) {
    sum_ptr[i] = this->_data[i] + other._data[i];
  }

  return Matrix<T>(this->_rows, this->_cols, ::std::move(sum_ptr));
}

template <typename T>
Matrix<T> Matrix<T>::operator+(T a) const noexcept {
  return element_wise(a, [] (T &b, T c) {
    b += c;
  });
}

template <typename T>
Matrix<T> Matrix<T>::operator-(const Matrix &other) const {
  if (this->_rows != other._rows || this->_cols != other._cols) {
    throw ::std::invalid_argument("Matrix addition error - misaligned dims");
  }

  size_t total_el = _rows * _cols;
  auto sum_ptr = ::std::make_unique<T[]>(total_el);

  for (size_t i = 0; i < total_el; i++) {
    sum_ptr[i] = this->_data[i] - other._data[i];
  }

  return Matrix<T>(this->_rows, this->_cols, ::std::move(sum_ptr));
}

template <typename T>
Matrix<T> Matrix<T>::operator-(T a) const noexcept {
  return element_wise(a, [] (T &b, T c) {
    b -= c;
  });
}

template <typename T>
Matrix<T> Matrix<T>::identity(size_t dim) {
  Matrix<T> res(dim, dim);
  T *res_ptr = res._data.get();
  
  for (size_t i = 0; i < dim; i++) {
    *res_ptr = 1;
    res_ptr += dim + 1;
  }

  return res;
}

template <typename T>
Matrix<T> Matrix<T>::abs() const {
  return element_wise(0, [] (T &val, T _) { val = ::std::abs(val); });
}

template <typename T>
T Matrix<T>::sum() const {
  return ::std::reduce(_data.get(),
		       _data.get() + _rows * _cols);
}

template <typename T>
T Matrix<T>::mean() const {
  if (_cols > 1) {
    throw ::std::invalid_argument("Mean error - mean only supported for 1 dimension");
  }
  
  return this->sum() / _rows;
}

template <typename T>
T Matrix<T>::std() const {
  if (_cols > 1) {
    ::std::invalid_argument("Std error - std only supported for 1 dimension");
  }
  
  T m = mean();
  T sd{ 0 };

  ::std::for_each(_data.get(), _data.get() + _rows, [&m, &sd] (T val) {
    T diff = val - m;
    sd += (diff * diff);
  });

  return sd / _rows;
}

template <typename T>
Matrix<T> Matrix<T>::squared() const {
  return element_wise(0, [] (T &a, T b) {
    a *= a;
  });
}

template <typename T>
Matrix<T> Matrix<T>::standardize() const {
  Matrix<T> res(_rows, _cols);
  
  for (size_t i = 0; i < _cols; i++) {
    auto c = get(COL, i);

    T m = c.mean();
    T sd = c.std();

    T *res_ptr = res._data.get() + i;
    T *this_ptr = _data.get() + i;

    for (size_t i = 0; i < _rows; i++) {
      *res_ptr = (*this_ptr - m) / sd;
      
      this_ptr += _cols;
      res_ptr += _cols;
    }
  }

  return res;
}

// --------------
// Masking
// --------------

template <typename T>
BinaryMask Matrix<T>::operator<(T val) const {
  return BinaryMask::create_binary_mask_matrix< T, ::std::less<T> >(*this, val);
}

template <typename T>
BinaryMask Matrix<T>::operator<=(T val) const {
  return BinaryMask::create_binary_mask_matrix< T, ::std::less_equal<T> >(*this, val);
}

template <typename T>
BinaryMask Matrix<T>::operator>(T val) const {
  return BinaryMask::create_binary_mask_matrix< T, ::std::greater<T> >(*this, val);
}

template <typename T>
BinaryMask Matrix<T>::operator>=(T val) const {
  return BinaryMask::create_binary_mask_matrix< T, ::std::greater_equal<T> >(*this, val);
}

template <typename T>
BinaryMask Matrix<T>::operator==(T val) const {
  return BinaryMask::create_binary_mask_matrix< T, ::std::equal_to<T> >(*this, val);
}

template <typename T>
BinaryMask Matrix<T>::operator!=(T val) const {
  return BinaryMask::create_binary_mask_matrix< T, ::std::not_equal_to<T> >(*this, val);
}

template <typename T>
Matrix<T> Matrix<T>::col_wise_mask_application(const IndexMask &idx_mask) const noexcept {
  return idx_mask.matrix_apply_mask_col_wise< T >(*this);
}

template <typename T>
T Matrix<T>::operator[](size_t idx) const {
  if (_cols > 1)
    ::std::invalid_argument("Matrix indexing error - this function is only for Matrices of shape (n, 1)");

  if (idx > _rows)
    ::std::invalid_argument("Matrix indexing error - idx out of bounds");

  return _data[idx];
}

template <typename T>
T Matrix<T>::get(size_t row, size_t col) const {
  if (row >= _rows || row < 0 || col >= _cols || col < 0) {
    throw ::std::out_of_range("Matrix indexing error - dims out of bounds\n Matrix dims: (" + std::to_string(_rows) + ", " + std::to_string(_cols) + ")" + "\n" + "Row idx: " + ::std::to_string(row) + "\n" + "Col idx: " + ::std::to_string(col) + "\n");
    exit(1);
  }

  return _data[row * _cols + col];
}

template <typename T>
Matrix<T> Matrix<T>::get(Dim d, size_t idx) const {
  if (d == ROW) {
    if (idx >= _rows) {
      throw ::std::out_of_range("Row indexing error - index out of bounds");
    }

    auto res_ptr = ::std::make_unique<T[]>(_cols);
    ::std::copy(_data.get() + idx * _cols, _data.get() + (idx + 1) * _cols, res_ptr.get());
    return Matrix<T>(_cols, 1, ::std::move(res_ptr));
      
  } else {
    
    if (idx > _cols || idx < 0) {
      throw ::std::out_of_range("Column indexing error - index out of bounds");
    }

    auto res = ::std::make_unique<T[]>(_rows);
    
    size_t total_el = _rows;
    constexpr uint8_t n_threads = 5;
    
    T *res_ptr = res.get();
    T *this_ptr = _data.get();
    
    for (size_t i = 0; i < total_el; i++) {
      res_ptr[i] = this_ptr[i * _cols + idx];
    }

    return Matrix<T>(_rows, 1, ::std::move(res));
    
  }
}

template <typename T>
CNum::DataStructs::Views::StrideView<T> Matrix<T>::get_col_view(size_t idx) const {
  if (idx > _cols || idx < 0) {
    throw ::std::out_of_range("Column indexing error - index out of bounds");
  }

  return CNum::DataStructs::Views::StrideView<T>(_data.get() + idx, _cols, _rows);
}

template <typename T>
::std::span<T> Matrix<T>::get_row_view(size_t idx) const {
  if (idx >= _rows) {
    throw ::std::out_of_range("Row indexing error - index out of bounds");
  }

  return std::span<T>(_data.get() + (_cols * idx), _cols);
}

template <typename T>
Matrix<T> Matrix<T>::operator[](const BinaryMask &m) const {
  return m.mask(*this);
}

template <typename T>
Matrix<T> Matrix<T>::operator[](const IndexMask &idx_mask) const noexcept {
  return idx_mask.matrix_apply_mask< T >(*this);
}

template <typename T>
IndexMask Matrix<T>::argsort(bool descending) const {
  if (descending)
    return IndexMask::argsort< Matrix<T>, T, ::std::greater<T> >(*this);

  return IndexMask::argsort< Matrix<T>, T, ::std::less<T> >(*this);
}

// -------------------
// Transformations
// -------------------

template <typename T>
Matrix<T> Matrix<T>::transpose() const noexcept {
  Matrix<T> res(_cols, _rows);
  T *this_ptr = _data.get();
  
  for (size_t i = 0; i < _rows; i++) {
    for (size_t j = 0; j < _cols; j++) {
      res._data[j * _rows + i] = *this_ptr;
      this_ptr++;
    }
  }

  return res;
}


//----------------
// Data mgmt
//----------------

template <typename T>
Matrix<T> Matrix<T>::join_cols(::std::vector< Matrix<T> > &cols) {
  size_t res_cols = cols.size();
  size_t res_rows = cols[0].get_rows();
  auto res_ptr = ::std::make_unique<T[]>(res_cols * res_rows);

  for (size_t i = 0; i < res_cols; i++) {
    for (size_t j = 0; j < res_rows; j++) {
      res_ptr[j * res_cols + i] = cols[i].get(j, 0);
    }
  }

  return Matrix<T>(res_rows, res_cols, ::std::move(res_ptr));
}

template <typename T>
Matrix<T> Matrix<T>::combine_vertically(::std::vector< Matrix<T> > &matrices, size_t total_rows) {
  size_t cols = matrices[0].get_cols();

  auto res_data = ::std::make_unique<T[]>(total_rows * cols);
  size_t res_pos{ 0 };
  for (auto &m: matrices) {
    ::std::move(m.begin(), m.end(), res_data.get() + res_pos);
    res_pos += m.get_rows() * cols;
  }

  return Matrix<T>(total_rows, cols, ::std::move(res_data));
}

template <typename T>
Matrix<T> Matrix<T>::init_const(size_t rows, size_t cols, T val) {
  auto res = ::std::make_unique<T[]>(rows * cols);
  ::std::fill(res.get(), res.get() + rows * cols, val);
  return Matrix<T>(rows, cols, ::std::move(res));
}

template <typename T>
size_t Matrix<T>::get_rows() const { return _rows; }

template <typename T>
size_t Matrix<T>::get_cols() const { return _cols; }

template <typename T>
T *Matrix<T>::begin() { return _data.get(); }

template <typename T>
T *Matrix<T>::end() { return _data.get() + _rows * _cols; }

template <typename T>
const T *Matrix<T>::begin() const { return _data.get(); }

template <typename T>
const T *Matrix<T>::end() const { return _data.get() + _rows * _cols; }

template <typename T>
size_t Matrix<T>::size() const { return _rows; }

template <typename T>
::std::unique_ptr<T[]> &&Matrix<T>::move_ptr() {
  return ::std::move(_data);
}

template <typename T>
void Matrix<T>::print_matrix() const {
  T *ptr = _data.get();
  for (size_t i = 0; i < _rows; i++) {
    for (size_t j = 0; j < _cols; j++) {
      ::std::cout << *ptr << " ";
      ptr += 1;
    }
    ::std::cout << ::std::endl;
  }
}
