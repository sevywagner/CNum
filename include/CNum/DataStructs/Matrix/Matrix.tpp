#include <utility>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <execution>

//---------------
// Constructors
//---------------

// ---- Default Overloaded Constructor ----
template <typename T>
Matrix<T>::Matrix(size_t rows, size_t cols, ::std::unique_ptr<T[]> ptr)
  : _cols(cols), _rows(rows), _data(::std::move(ptr))  {
  if (_data == nullptr && _rows > 0 && _cols > 0) {
    _data = ::std::make_unique<T[]>(_rows * _cols);
  }
}

// ---- Copy Logic ----
template <typename T>
void Matrix<T>::copy(const Matrix &other) {
  if (this == &other) return;

  this->_rows = other._rows;
  this->_cols = other._cols;

  if (this->_data != nullptr)
    this->_data.reset();
  
  this->_data = ::std::make_unique<T[]>(other._rows * other._cols);
  ::std::copy(other._data.get(), other._data.get() + other._rows * other._cols, this->_data.get());
}

// ---- Copy Constructor ----
template <typename T>
Matrix<T>::Matrix(const Matrix &other) noexcept {
  this->copy(::std::cref(other));
}

// ---- Copy Assignment ----
template <typename T>
Matrix<T> &Matrix<T>::operator=(const Matrix &other) noexcept {
  this->copy(::std::cref(other));
  return *this;
}

// ---- Move Logic ----
template <typename T>
void Matrix<T>::move(Matrix<T> &&other) {
  if (this == &other) {
    return;
  }

  this->_rows = other._rows;
  other._rows = 0;
  this->_cols = other._cols;
  other._cols = 0;

  this->_data = ::std::move(other._data);
}

// ---- Move Constructor ----
template <typename T>
Matrix<T>::Matrix(Matrix &&other) noexcept {
  this->move(::std::move(other));
}

// ---- Move Assignment ----
template <typename T>
Matrix<T> &Matrix<T>::operator=(Matrix &&other) noexcept {
  this->move(::std::move(other));
  return *this;
}

// ---- Destructor ----
template <typename T>
Matrix<T>::~Matrix() {}

//-----------------
// Linear Algebra
//-----------------

// ---- Parrallel element wise operation abstraction ----
template <typename T>
Matrix<T> Matrix<T>::element_wise(T val, ::std::function< void(T &, T) > func) const noexcept {
  Matrix<T> res(*this);
  
  ::std::for_each(::std::execution::par_unseq,
		  res._data.get(),
		  res._data.get() + res._rows * res._cols,
		  [&func, &val] (T &t) {
		    func(t, val);
		  });

  return res;
}


// ---- Dot Product ----
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

// ---- Scale ----
template <typename T>
Matrix<T> Matrix<T>::operator*(T scale_factor) const noexcept {
  return element_wise(scale_factor, [] (T &a, T b) {
    a *= b;
  });
}

// ---- Vector dot product ----
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

// ---- Parrellel Execution Abstraction ----
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

// ---- Matrix Addition ----
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

// ---- Element wise value addition ----
template <typename T>
Matrix<T> Matrix<T>::operator+(T a) const noexcept {
  return element_wise(a, [] (T &b, T c) {
    b += c;
  });
}

// ---- Matrix Subtraction ----
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

// ---- Element wise value subtraction ----
template <typename T>
Matrix<T> Matrix<T>::operator-(T a) const noexcept {
  return element_wise(a, [] (T &b, T c) {
    b -= c;
  });
}

// ---- Identity Matrix ----
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

// ---- Absolute value ----
template <typename T>
Matrix<T> Matrix<T>::abs() const {
  return element_wise(0, [] (T &val, T _) { val = ::std::abs(val); });
}

// ---- Sum ----
template <typename T>
T Matrix<T>::sum() const {
  return ::std::reduce(::std::execution::seq,
		       _data.get(),
		       _data.get() + _rows * _cols);
}

// ---- Mean ----
template <typename T>
T Matrix<T>::mean() const {
  if (_cols > 1) {
    throw ::std::invalid_argument("Mean error - mean only supported for 1 dimension");
  }
  
  return this->sum() / _rows;
}

// ---- Standard Deviation ----
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

// ---- Element Wise Square ----
template <typename T>
Matrix<T> Matrix<T>::squared() const {
  return element_wise(0, [] (T &a, T b) {
    a *= a;
  });
}

// ---- Standardize Matrix ----
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

// ---- Create Mask ----
template <typename T>
Mask<BIN, bool> Matrix<T>::comparison_mask(::std::function< bool(const T &a, const T &b) > comp, T val) const {
  if (_cols > 1) {
    throw ::std::invalid_argument("Comparison mask only supported for 1 dimension i.e. Matrix<T>(x, 1)");
  }

  auto res = ::std::make_unique<bool[]>(_rows);
  ::std::atomic<int> num_pos = 0;

  size_t total_el = _rows;
  uint8_t n_threads = 5;

  par_execute(n_threads, total_el, [res_ptr = res.get(),
				    data_ptr = _data.get(),
				    &num_pos,
				    comp,
				    val] (size_t i) {
    bool comp_res = comp(data_ptr[i], val);
    res_ptr[i] = comp_res;

    if (comp_res) {
      num_pos++;
    }
  });

  return Mask<BIN, bool>(_rows, num_pos, ::std::move(res));
}

template <typename T>
Mask<BIN, bool> Matrix<T>::operator<(T val) const {
  return comparison_mask([] (const T &a, const T &b) { return a < b; }, val);
}

template <typename T>
Mask<BIN, bool> Matrix<T>::operator<=(T val) const {
  return comparison_mask([] (const T &a, const T &b) { return a <= b; }, val);
}

template <typename T>
Mask<BIN, bool> Matrix<T>::operator>(T val) const {
  return comparison_mask([] (const T &a, const T &b) { return a > b; }, val);
}

template <typename T>
Mask<BIN, bool> Matrix<T>::operator>=(T val) const {
  return comparison_mask([] (const T &a, const T &b) { return a >= b; }, val);
}

template <typename T>
Mask<BIN, bool> Matrix<T>::operator==(T val) const {
  return comparison_mask([] (const T &a, const T &b) { return a == b; }, val);
}

template <typename T>
Mask<BIN, bool> Matrix<T>::operator!=(T val) const {
  return comparison_mask([] (const T &a, const T &b) { return a != b; }, val);
}

// ---- Apply index mask column wise ----
template <typename T>
Matrix<T> Matrix<T>::col_wise_mask_application(const Mask<IDX, uint32_t> &idx_mask) const noexcept {
  auto masked_ptr = ::std::make_unique<T[]>(idx_mask.get_len() * _rows);
  auto *masked_ptr_raw = masked_ptr.get();
  
  for (size_t i = 0; i < idx_mask.get_len(); i++) {
    size_t idx = idx_mask[i];
    auto col_view = this->get_col_view(idx);
    auto col_view_it = col_view.begin();
      
    for (size_t j = 0; j < _rows; j++) {
      masked_ptr_raw[j * idx_mask.get_len() + i] = *col_view_it;
      ++col_view_it;
    }
  }

  return Matrix<T>(_rows, idx_mask.get_len(), ::std::move(masked_ptr));
}

// ---- Indexing ----
template <typename T>
T Matrix<T>::get(size_t row, size_t col) const {
  if (row >= _rows || row < 0 || col >= _cols || col < 0) {
    throw ::std::out_of_range("Matrix indexing error - dims out of bounds\n Matrix dims: (" + std::to_string(_rows) + ", " + std::to_string(_cols) + ")" + "\n" + "Row idx: " + ::std::to_string(row) + "\n" + "Col idx: " + ::std::to_string(col) + "\n");
    exit(1);
  }

  return _data[row * _cols + col];
}

// ---- Get Row/Col ----
template <typename T>
Matrix<T> Matrix<T>::get(enum dim d, size_t idx) const {
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

// ---- Apply Binary Mask ----
template <typename T>
Matrix<T> Matrix<T>::operator[](const Mask<BIN, bool> &m) const {
  if (m.get_len() != _rows) {
    throw ::std::out_of_range("Binary mask application error -- Misaligned dims");
  }
  
  auto res = ::std::make_unique<T[]>(m.get_num_positive() * _cols);
  T *res_ptr = res.get();
  T *this_ptr = _data.get();
  const bool *mask_ptr = m.get_ptr();
  
  for (size_t i = 0; i < m.get_len(); i++) {
    if (*mask_ptr) {
      ::std::copy(this_ptr, this_ptr + _cols, res_ptr);
      res_ptr += _cols;
    }

    this_ptr += _cols;
    mask_ptr++;
  }

  return Matrix<T>(m.get_num_positive(), _cols, ::std::move(res));
}

// ---- Apply Idx Mask ----
template <typename T>
Matrix<T> Matrix<T>::operator[](const Mask<IDX, uint32_t> &idx_mask) const noexcept {
  int mask_len = idx_mask.get_len();
  auto res = ::std::make_unique<T[]>(mask_len * _cols);

  for (size_t i = 0; i < mask_len; i++) {
    std::copy(_data.get() + (idx_mask[i]) * _cols,
	      _data.get() + (idx_mask[i] + 1) * _cols,
	      res.get() + i * _cols);
  };
  
  return Matrix<T>(mask_len, _cols, ::std::move(res));
}


// ---- Argsort ----
template <typename T>
Mask<IDX, uint32_t> Matrix<T>::argsort() const {
  if (_cols != 1) {
    throw ::std::invalid_argument("Argsort error - argsort only supported for 1 dimension");
  }
  
  auto mask = ::std::make_unique<uint32_t[]>(_rows);
  ::std::iota(mask.get(), mask.get() + _rows, 0);

  ::std::sort(::std::execution::par, mask.get(), mask.get() + _rows, [&] (uint32_t a, uint32_t b) {
    return _data[a] < _data[b];
  });

  return Mask<IDX, uint32_t>(_rows, 0, ::std::move(mask));
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
Matrix<T> Matrix<T>::init_const(size_t rows, size_t cols, T val) {
  auto res = ::std::make_unique<T[]>(rows * cols);
  ::std::fill(::std::execution::par_unseq, res.get(), res.get() + rows * cols, val);
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
