#ifndef __MASK_H
#define __MASK_H

#include <memory>
#include <thread>
#include <vector>
#include <execution>
#include <iostream>
#include <random>
#include <ctime>
#include <mutex>

namespace CNum::DataStructs {
  enum MaskType {
    IDX,
    BIN
  };

  template <enum MaskType MT, typename PT>
  class Mask {
  private:
    size_t _len;
    size_t _num_ones;
    ::std::unique_ptr<PT[]> _mask;

    void copy(const Mask &other);
    void move(Mask &&other);

  public:
    Mask(size_t len = 0, size_t n_ones = 0, ::std::unique_ptr<PT[]> m = nullptr);
    
    Mask(const Mask &other);
    Mask<MT, PT> &operator=(const Mask &other) noexcept;
  
    Mask(Mask &&other);
    Mask<MT, PT> &operator=(Mask &&other) noexcept;
  
    ~Mask();
    
    size_t get_num_positive() const;
    size_t get_len() const;

    PT operator[](int idx) const noexcept;
    ::std::unique_ptr<PT[]> &&move_mask();
    const PT *get_ptr() const;

    static Mask<IDX, uint32_t> gen_rand_mask(int start = 0, int end = 10, int n = 10);
    static Mask<BIN, bool> gen_rand_bin_mask(int start = 0, int end = 10, int n = 10, int len = 10);
  
    Mask<IDX, uint32_t> operator[](const Mask<BIN, bool> &other) const;
    static Mask<BIN, bool> flip(const Mask<BIN, bool> &other);
  
    void print_mask() const;
  };

#include "Mask.tpp"
};

#endif
