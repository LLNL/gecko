#ifndef GECKO_SUBGRAPH_H
#define GECKO_SUBGRAPH_H

#include "gecko/graph.h"
#include "options.h"

namespace Gecko {

// Node in a subgraph.
class Subnode {
public:
  typedef unsigned char Index;
  Float pos;        // node position
  WeightedSum cost; // external cost at this position
};

class Subgraph {
public:
  Subgraph(Graph* g, uint n);
  ~Subgraph() { delete[] cache; }
  void optimize(uint k);

private:
  Graph* const g;                        // full graph
  const uint n;                          // number of subgraph nodes
  Functional* const f;                   // ordering functional
  WeightedSum min;                       // minimum cost so far
  Subnode::Index best[GECKO_WINDOW_MAX]; // best permutation so far
  Subnode::Index perm[GECKO_WINDOW_MAX]; // current permutation
  const Subnode* node[GECKO_WINDOW_MAX]; // pointers to precomputed nodes
  Subnode* cache;                        // precomputed node positions and costs
#if GECKO_SUBGRAPH_ADJLIST
  Subnode::Index adj[GECKO_WINDOW_MAX][GECKO_WINDOW_MAX]; // internal adjacency list
#else
  uint adj[GECKO_WINDOW_MAX];            // internal adjacency matrix
#endif
  Float weight[GECKO_WINDOW_MAX][GECKO_WINDOW_MAX]; // internal arc weights
  WeightedSum cost(uint k) const;
  void swap(uint k);
  void swap(uint k, uint l);
  void optimize(WeightedSum c, uint i);
};

}

#endif
