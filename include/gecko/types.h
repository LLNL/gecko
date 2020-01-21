#ifndef GECKO_TYPES_H
#define GECKO_TYPES_H

#include <cfloat>
#include <limits>

#define GECKO_FLOAT_EPSILON std::numeric_limits<Float>::epsilon()
#define GECKO_FLOAT_MAX std::numeric_limits<Float>::max()

namespace Gecko {

  typedef unsigned int uint;

// precision for node positions and computations
#if GECKO_WITH_DOUBLE_PRECISION
  typedef double Float;
#else
  typedef float Float;
#endif
}

#endif
