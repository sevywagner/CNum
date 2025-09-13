#ifndef __DATA_H
#define __DATA_H

#include "CNum/DataStructs/DataStructs.h"

#include <string>
#include <memory>
#include <fstream>
#include <sstream>

namespace CNum::Data {
  struct bin {
    uint32_t num;
    size_t ct;

    bin() : ct(0) {}
  };

  struct shelf {
    size_t num_bins;
    std::unique_ptr<bin[]> bins;
    std::unique_ptr<double[]> ranges;

    shelf() : num_bins(0) {}
    shelf(size_t nb)
      : num_bins(nb),
	bins(std::make_unique<bin[]>(nb)),
	ranges(std::make_unique<double[]>(nb - 1)) {}

    shelf &operator=(struct shelf &&other) {
      if (this == &other)
	return *this;
    
      num_bins = other.num_bins;

      bins.reset();
      ranges.reset();
    
      bins = std::move(other.bins);
      ranges = std::move(other.ranges);

      return *this;
    }
  };
  
  std::array< CNum::DataStructs::Matrix<double>, 2 > get_data(std::string data_path, char seperator = ',');
  void PCA(std::string input_path, std::string output_path);
  std::shared_ptr<struct shelf[]> uniform_bin(const CNum::DataStructs::Matrix<double> &data, size_t num_bins = 256);
  std::shared_ptr<struct shelf[]> quantile_bin(const CNum::DataStructs::Matrix<double> &data, size_t num_bins = 256);
  CNum::DataStructs::Matrix<int> apply_quantile(const CNum::DataStructs::Matrix<double> &data, std::shared_ptr<struct shelf[]> shelves);
};

#endif
