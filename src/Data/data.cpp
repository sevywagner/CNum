#include "CNum/Data/Data.h"

using namespace CNum::DataStructs;

namespace CNum::Data {
  std::array< Matrix<double>, 2 > get_data(std::string data_path, char seperator) {
    std::ifstream is(data_path);
  
    if (!is.is_open()) {
      throw ::std::runtime_error("Get data error - Failed to open file");
    }

    std::array< Matrix<double>, 2 > data;
    std::vector<double> X;
    std::vector<double> y;

    std::string line;
    getline(is, line, '\n');
    int X_cols = std::count(line.begin(), line.end(), seperator);

    do {
      std::stringstream ss(line);
      std::string segment;

      for (int i = 0; i < X_cols; i++) {
	getline(ss, segment, seperator);
	try {
	  X.push_back(stof(segment));
	} catch(...) {
	  std::cout << "Error converting " << segment << " to float" << std::endl;
	}
      }

      getline(ss, segment, seperator);
      y.push_back(stod(segment));
    
    } while (getline(is, line, '\n'));
  
    is.close();

    auto X_ptr = std::make_unique<double[]>(X.size());
    auto y_ptr = std::make_unique<double[]>(y.size());

    std::copy(X.begin(), X.end(), X_ptr.get());
    std::copy(y.begin(), y.end(), y_ptr.get());

    data[0] = Matrix<double>(y.size(), X_cols, std::move(X_ptr));
    data[1] = Matrix<double>(y.size(), 1, std::move(y_ptr));
  
    return data;
  }

  void PCA(std::string input_path, std::string output_path) {
    // PCA coming soon
  }
  
  std::shared_ptr<Shelf[]> uniform_bin(const Matrix<double> &data, size_t num_bins) {
    std::shared_ptr<Shelf[]> shelves(new Shelf[data.get_cols()]);

    for (int i = 0; i < data.get_cols(); i++) {
      shelves[i] = Shelf(num_bins);
      auto col = data.get(COL, i);

      double min = col.get(0, 0);
      double max = col.get(0, 0);

      for (int j = 1; j < col.get_rows(); j++) {
	double val = col.get(j, 0);
      
	if (val < min) {
	  min = val;
	}

	if (val > max) {
	  max = val;
	}
      }
    
      double step_size = (max - min) / num_bins;
      double step = min;

      for (int j = 0; j < num_bins - 1; j++) {
	shelves[i].ranges[j] = step;
	step += step_size;
      }

      for (int j = 0; j < col.get_rows(); j++) {
	size_t b = std::min(static_cast<size_t>((col.get(j, 0) - min) / step_size), num_bins - 1);
	shelves[i].bins[b].ct++;
      }
    
    }

    return shelves;
  }

  
  std::shared_ptr<Shelf[]> quantile_bin(const Matrix<double> &data, size_t num_bins) {
    auto shelves = uniform_bin(std::cref(data), num_bins);
    std::shared_ptr<Shelf[]> new_shelves(new Shelf[data.get_cols()]);

    for (int i = 0; i < data.get_cols(); i++) {
      new_shelves[i] = Shelf(num_bins);
    
      auto cumulative_sum = std::make_unique<int[]>(num_bins);
      int sum = 0;

      for (int j = 0; j < num_bins; j++) {
	sum += shelves[i].bins[j].ct;
	cumulative_sum[j] = sum;
      }

      int uni = 0;
      for (int quant = 0; quant < num_bins - 1; quant++) {
	double quant_fraction = static_cast<double>(quant) / num_bins;
	int count = quant_fraction * data.get_rows();
	while (uni < num_bins - 1 && cumulative_sum[uni] < count) {
	  uni++;
	}

	double b_low = (uni == 0) ? shelves[i].ranges[0] : shelves[i].ranges[uni - 1];
	double b_high = (uni >= num_bins - 1) ? shelves[i].ranges[num_bins - 2] + 1e-3 : shelves[i].ranges[uni];
      
	int bin_ct = shelves[i].bins[uni].ct;
	double r = bin_ct == 0 ? 0 : (static_cast<double>(count - (uni == 0 ? 0 : cumulative_sum[uni - 1])) / bin_ct);
	double interpolated = b_low + r * (b_high - b_low);

	if (quant > 0) {
	  double prev = new_shelves[i].ranges[quant - 1];
	  if (interpolated < prev)
	    interpolated = prev;
	}

	new_shelves[i].ranges[quant] = interpolated;
      }
    }

    return new_shelves;
  }

  
  Matrix<int> apply_quantile(const Matrix<double> &data, std::shared_ptr<Shelf[]> shelves) {
    std::vector< Matrix<int> > cols;
    std::vector< std::future< Matrix<int> > > workers;
    workers.reserve(data.get_cols());

    auto *tp = CNum::Multithreading::ThreadPool::get_thread_pool();
    for (int i = 0; i < data.get_cols(); i++) {
      workers.push_back(tp->submit< Matrix<int> >([&, i] (arena_t *arena) {
	auto col = data.get(COL, i);
	auto binned_col = std::make_unique<int[]>(col.get_rows());
	
	for (int j = 0; j < col.get_rows(); j++) {
	  double val = col.get(j, 0);

	  if (val < shelves[i].ranges[0]) {
	    binned_col[j] = 0;
	    continue;
	  }

	  for (int k = 1; k < shelves[i].num_bins - 1; k++) {
	    if (val >= shelves[i].ranges[k - 1] && val < shelves[i].ranges[k]) {
	      binned_col[j] = k;
	      break;
	    }
	  }

	  if (val > shelves[i].ranges[shelves[i].num_bins - 2]) {
	    binned_col[j] = shelves[i].num_bins - 1;
	  }
	}

	return Matrix<int>(col.get_rows(), 1, std::move(binned_col));
      }));
    }

    for (auto &t: workers) {
      cols.push_back(t.get());
    }

    return Matrix<int>::join_cols(cols);
  }
};
