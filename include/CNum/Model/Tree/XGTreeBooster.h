#ifndef __XG_TREE_BOOSTER_H
#define __XG_TREE_BOOSTER_H

#include "CNum/Model/Tree/TreeBooster.h"
#include "CNum/Model/Tree/TreeBoosterNode.h"

namespace CNum::Model::Tree {
  class XGTreeBooster : public TreeBooster {
  private:
    virtual void fit_node_greedy(const CNum::DataStructs::Matrix<double> &X,
				 double *g,
				 double *h,
				 TreeBoosterNode *node,
				 int depth = 0) override;
    
    virtual void fit_node_hist(const CNum::DataStructs::Matrix<int> &X,
			       std::shared_ptr<struct CNum::Data::shelf[]> shelves,
			       double *g,
			       double *h,
			       DataPartition &partition,
			       const arena_view_t &parent_hist_view,
			       TreeBoosterNode *node,
			       int depth = 0) override;

    virtual void fit_prep(const CNum::DataStructs::Matrix<int> &X,
			  std::shared_ptr<struct CNum::Data::shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) override;

    virtual void fit_prep(const CNum::DataStructs::Matrix<double> &X,
			  std::shared_ptr<struct CNum::Data::shelf[]> shelves,
			  double *g,
			  double *h,
			  DataPartition &partition) override;
  
  public:
    XGTreeBooster(arena_t *a = nullptr,
		  int md = 5,
		  int ms = 3,
		  double weight_decay = 0.0,
		  double reg_lambda = 1.0,
		  double gamma = 0.0);

    ~XGTreeBooster();

    virtual void fit(DataMatrix &X,
		     std::shared_ptr<struct CNum::Data::shelf[]> shelves,
		     double *g,
		     double *h,
		     DataPartition &partition) override;
  };
};

#endif
