#include "CNum/DataStructs/Matrix/Mask.h"

// --------------
// Constructors
// --------------

// ---- Overloaded ----
template <enum MaskType MT, typename PT>
Mask<MT, PT>::Mask(size_t len, size_t n_ones, ::std::unique_ptr<PT[]> m)
  : _len(len), _num_ones(n_ones), _mask(::std::move(m)) {
  if (_mask == nullptr && _len > 0) {
    _mask = ::std::make_unique<PT[]>(_len);
  }
}

// ---- Copy ----
template <enum MaskType MT, typename PT>
void Mask<MT, PT>::copy(const Mask &other) {
  if (this == &other) {
    return;
  }

  this->_len = other._len;
  this->_num_ones = other._num_ones;

  if (this->_mask != nullptr)
    _mask.reset();

  this->_mask = ::std::make_unique<PT[]>(other._len);
  
  ::std::copy(other._mask.get(), other._mask.get() + other._len, this->_mask.get());
}

template <enum MaskType MT, typename PT>
Mask<MT, PT>::Mask(const Mask &other) {
  copy(::std::cref(other));
}

template <enum MaskType MT, typename PT>
Mask<MT, PT> &Mask<MT, PT>::operator=(const Mask &other) noexcept {
  copy(::std::cref(other));
  return *this;
}

// ---- Move ----
template <enum MaskType MT, typename PT>
void Mask<MT, PT>::move(Mask &&other) {
  if (this == &other) {
    return;
  }

  this->_len = other._len;
  other._len = 0;
  this->_num_ones = other._num_ones;
  other._num_ones = 0;

  this->_mask = ::std::move(other._mask);
}

template <enum MaskType MT, typename PT>
Mask<MT, PT>::Mask(Mask &&other) {
  move(::std::move(other));
}

template <enum MaskType MT, typename PT>
Mask<MT, PT> &Mask<MT, PT>::operator=(Mask &&other) noexcept {
  move(::std::move(other));
  return *this;
}

template <enum MaskType MT, typename PT>
Mask<MT, PT>::~Mask() {}


// -----------
// Getters
// -----------

template <enum MaskType MT, typename PT>
size_t Mask<MT, PT>::get_num_positive() const { return _num_ones; }

template <enum MaskType MT, typename PT>
size_t Mask<MT, PT>::get_len() const { return _len; }

// ---- indexing ----
template <enum MaskType MT, typename PT>
PT Mask<MT, PT>::operator[](int idx) const noexcept {
  return _mask[idx];
}

// --------------
// Pointer ops
// --------------

// ---- Transfer ownership of mask ----
template <enum MaskType MT, typename PT>
::std::unique_ptr<PT[]> &&Mask<MT, PT>::move_mask() { return ::std::move(_mask); }

// ---- Return pointer without transferring ownership ----
template <enum MaskType MT, typename PT>
const PT *Mask<MT, PT>::get_ptr() const { return _mask.get(); }


// ------------
// Rand masks
// ------------

// ---- Random index mask ----
template <enum MaskType MT, typename PT>
Mask<IDX, uint32_t> Mask<MT, PT>::gen_rand_mask(int start, int end, int n) {
  ::std::random_device rd;
  ::std::mt19937 generator(rd());
  ::std::uniform_int_distribution<> distribution(start, end);
  ::std::unordered_map<int, int> idx_used;
  
  auto mask_ptr = ::std::make_unique<uint32_t[]>(n);
  for (int i = 0; i < n; i++) {
    int rand = distribution(generator);
    if (idx_used[rand] > 0) continue;

    mask_ptr[i] = rand;
    idx_used[rand]++;
  };

  return Mask<IDX, uint32_t>(n, 0, ::std::move(mask_ptr));
}

// ---- Random binary mask ----
template <enum MaskType MT, typename PT>
Mask<BIN, bool> Mask<MT, PT>::gen_rand_bin_mask(int start, int end, int n, int len) {
  ::std::random_device rd;
  ::std::mt19937 generator(rd());
  ::std::uniform_int_distribution<> distribution(start, end);
  ::std::unordered_map<int, int> idx_used;
  auto mask = ::std::make_unique<bool[]>(n);

  while (n-- >= 0) {
    int rand = distribution(generator);
    if (idx_used[rand] > 0) continue;
    
    mask[rand] = true;
    idx_used[rand]++;
  }

  return Mask<BIN, bool>(len, n, ::std::move(mask));
}

// -----------
// Mutations
// -----------

// ---- Binary mask to index mask application -----
template <enum MaskType MT, typename PT>
Mask<IDX, uint32_t> Mask<MT, PT>::operator[](const Mask<BIN, bool> &other) const {
  if (this->_len != other.get_len()) {
    throw ::std::invalid_argument("Binary mask application error - dimensions misaligned");
  }
  
  auto res = ::std::make_unique<uint32_t[]>(other.get_num_positive());
  auto *res_ptr = res.get();
  const bool *mask_ptr = other.get_ptr();

  for (int i = 0; i < this->_len; i++) {
    if (*mask_ptr) {
      *res_ptr = this->_mask[i];
      res_ptr++;
    }
    mask_ptr++;
  }
  
  return Mask<IDX, uint32_t>(other.get_num_positive(), 0, ::std::move(res));
}

// ---- Flip binary mask bits ----
template <enum MaskType MT, typename PT>
Mask<BIN, bool> Mask<MT, PT>::flip(const Mask<BIN, bool> &other) {
  auto res = other;
  
  ::std::for_each(::std::execution::par_unseq,
		res._mask.get(),
		res._mask.get() + res._len,
		[] (bool &val) {
		  val = !val;
		});

  res._num_ones = other._len - other._num_ones;
  return res;
}

// ---- Print ----
template <enum MaskType MT, typename PT>
void Mask<MT, PT>::print_mask() const {
  PT *m_ptr = _mask.get();
  for (int i = 0; i < _len; i++) {
    ::std::cout << *m_ptr << " ";
    m_ptr++;
  }
  ::std::cout << ::std::endl;
}
