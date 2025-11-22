#ifndef VIEWS_H
#define VIEWS_H

#include "CNum/DataStructs/Views/StrideView.h"

/**
 * @namespace CNum::DataStructs::Views
 * @brief The views for CNum DataStructs
 *
 * Views are a structures consisting of a pointer and a range
 */
namespace CNum::DataStructs::Views {
  template <typename T>
  class StrideIterator;

  template <typename T>
  class StrideView;
}

#endif
