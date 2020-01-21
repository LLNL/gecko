#ifndef GECKO_OPTIONS_H
#define GECKO_OPTIONS_H

#include <cfloat>
#include <limits>

// ratio of max to min weight for aggregation
#ifndef GECKO_PART_FRAC
  #define GECKO_PART_FRAC 4
#endif

// number of compatible relaxation sweeps
#ifndef GECKO_CR_SWEEPS
  #define GECKO_CR_SWEEPS 1
#endif

// number of Gauss-Seidel relaxation sweeps
#ifndef GECKO_GS_SWEEPS
  #define GECKO_GS_SWEEPS 1
#endif

// max number of nodes in subgraph
#ifndef GECKO_WINDOW_MAX
  #define GECKO_WINDOW_MAX 16
#endif

// use adjacency list (1) or adjacency matrix (0)
#ifndef GECKO_WITH_ADJLIST
  #define GECKO_WITH_ADJLIST 0
#endif

// use nonrecursive permutation algorithm
#ifndef GECKO_WITH_NONRECURSIVE
  #define GECKO_WITH_NONRECURSIVE 0
#endif

// use double-precision computations
#ifndef GECKO_WITH_DOUBLE_PRECISION
  #define GECKO_WITH_DOUBLE_PRECISION 0
#endif

#endif
