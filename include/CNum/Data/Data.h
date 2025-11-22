#ifndef DATA_H
#define DATA_H

#include "CNum/DataStructs/DataStructs.h"

#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <array>

/**
 * @namespace CNum::Data
 * @brief Tools used for gathering and grouping datasets
 */
namespace CNum::Data {
  /**
   * @struct Bin 
   * @brief A bin for quantile and uniform binning
   */
  struct Bin {
    uint32_t num;
    size_t ct;
    Bin() : ct(0) {}
  };

  /**
   * @struct Shelf
   * @brief Contains bins and the ranges of values they represent
   */
  struct Shelf {
    size_t num_bins;
    std::unique_ptr<Bin[]> bins;
    std::unique_ptr<double[]> ranges;

    Shelf() : num_bins(0) {}
    Shelf(size_t nb)
      : num_bins(nb),
	bins(std::make_unique<Bin[]>(nb)),
	ranges(std::make_unique<double[]>(nb - 1)) {}

    Shelf &operator=(Shelf &&other) {
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
  
  /// @brief Get data from a _SV file with last column being the labels
  /// @param data_path The path to the data file
  /// @param seperator The delimiter of the columns
  /// @return The data and labels [data, labels]
  std::array< CNum::DataStructs::Matrix<double>, 2 > get_data(std::string data_path, char seperator = ',');

  /// @brief Principle component analysis
  ///
  /// Available next release
  void PCA(std::string input_path, std::string output_path);

  /// @brief Uniform binning of data
  /// @param data The dataset
  /// @param num_bins The number of bins to distribute the data among
  /// @return The bins and the boundaries associated with them
  std::shared_ptr<Shelf[]> uniform_bin(const CNum::DataStructs::Matrix<double> &data, size_t num_bins = 256);

  /// @brief Quantile sketch *not exact quantile bins*
  ///
  /// As perfect quantile binning is an expensive operation
  /// I used a quantile binning approximation
  /// @param data The dataset
  /// @param num_bins The number of bins to distribute the data among
  /// @return The bins and the boundaries associated with them
  std::shared_ptr<Shelf[]> quantile_bin(const CNum::DataStructs::Matrix<double> &data, size_t num_bins = 256);

  /// @brief Construct data matrix of bin values
  /// @param data The dataset
  /// @param shelves The bins and the boundaries associated with them
  /// @return The matrix of bin values
  CNum::DataStructs::Matrix<int> apply_quantile(const CNum::DataStructs::Matrix<double> &data, std::shared_ptr<Shelf[]> shelves);
};

#endif
