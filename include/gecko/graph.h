// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and other
// gecko project contributors. See the top-level LICENSE file for details.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GECKO_GRAPH_H
#define GECKO_GRAPH_H

#include <cmath>
#include <string>
#include <utility>
#include <vector>
#include "gecko/types.h"
#include "gecko/functional.h"
#include "gecko/progress.h"

namespace Gecko {

// Multilevel graph arc.
class Arc {
public:
  typedef uint Index;
  typedef std::vector<Index>::const_iterator ConstPtr;
  enum { null = 0 };
};

// Multilevel graph node.
class Node {
public:
  typedef uint Index;
  typedef std::vector<Node>::const_iterator ConstPtr;
  enum { null = 0 };

  // comparator for sorting node indices
  class Comparator {
  public:
    Comparator(ConstPtr node) : _node(node) {}
    bool operator()(uint k, uint l) const { return _node[k].pos < _node[l].pos; }
  private:
    const ConstPtr _node;
  };

  // constructor
  Node(Float pos = -1, Float length = 1, Arc::Index arc = Arc::null, Node::Index parent = Node::null) : pos(pos), hlen(Float(0.5) * length), arc(arc), parent(parent) {}

  Float pos;          // start position at full resolution
  Float hlen;         // half of node length (number of full res nodes)
  Arc::Index arc;     // one past index of last incident arc
  Node::Index parent; // parent in next coarser resolution
};

// Multilevel graph.
class Graph {
public:
  // constructor of graph with given (initial) number of nodes
  Graph(uint nodes = 0) : level(0), last_node(Node::null) { init(nodes); }

  // number of nodes and edges
  uint nodes() const { return node.size() - 1; }
  uint edges() const { return (adj.size() - 1) / 2; }

  // insert node and return its index
  Node::Index insert_node(Float length = 1);

  // outgoing arcs {begin, ..., end-1} originating from node i
  Arc::Index node_begin(Node::Index i) const { return node[i - 1].arc; }
  Arc::Index node_end(Node::Index i) const { return node[i].arc; }

  // node degree and neighbors
  uint node_degree(Node::Index i) const { return node_end(i) - node_begin(i); }
  std::vector<Node::Index> node_neighbors(Node::Index i) const;

  // insert directed edge (i, j)
  Arc::Index insert_arc(Node::Index i, Node::Index j, Float w = 1, Float b = 1);

  // remove arc or edge
  bool remove_arc(Arc::Index a);
  bool remove_arc(Node::Index i, Node::Index j);
  bool remove_edge(Node::Index i, Node::Index j);

  // index of arc (i, j) or null if not present
  Arc::Index arc_index(Node::Index i, Node::Index j) const;

  // arc source and target nodes and weight
  Node::Index arc_source(Arc::Index a) const;
  Node::Index arc_target(Arc::Index a) const { return adj[a]; }
  Float arc_weight(Arc::Index a) const { return weight[a]; }

  // reverse arc (j, i) of arc a = (i, j)
  Arc::Index reverse_arc(Arc::Index a) const;

  // order graph
  void order(Functional* functional, uint iterations = 1, uint window = 2, uint period = 2, uint seed = 0, Progress* progress = 0);

  // optimal permutation found
  const std::vector<Node::Index>& permutation() const { return perm; }

  // node of given rank in reordered graph (0 <= rank <= nodes() - 1)
  Node::Index permutation(uint rank) const { return perm[rank]; }

  // position of node i in reordered graph (1 <= i <= nodes())
  uint rank(Node::Index i) const { return static_cast<uint>(std::floor(node[i].pos)); }

  // cost of current layout
  Float cost() const;

  // return first directed arc if one exists or null otherwise
  Arc::Index directed() const;

protected:
  friend class Subgraph;
  friend class Drawing;

  // constructor/destructor
  Graph(uint nodes, uint level) : level(level), last_node(Node::null) { init(nodes); }

  // arc length
  Float length(Node::Index i, Node::Index j) const { return std::fabs(node[i].pos - node[j].pos); }
  Float length(Arc::Index a) const
  {
    Node::Index i = arc_source(a);
    Node::Index j = arc_target(a);
    return length(i, j);
  }

  // coarsen graph
  Graph* coarsen();

  // refine graph
  void refine(const Graph* graph);

  // perform m sweeps of compatible or Gauss-Seidel relaxation
  void relax(bool compatible, uint m = 1);

  // optimize using n-node window
  void optimize(uint n);

  // place all nodes according to their positions
  void place(bool sort = false);

  // place nodes {k, ..., k + n - 1} according to their positions
  void place(bool sort, uint k, uint n);

  // perform V cycle using n-node window
  void vcycle(uint n, uint work = 0);

  // randomly shuffle nodes
  void shuffle(uint seed = 0);

  // recompute arc bonds for iteration i
  void reweight(uint i);

  // compute cost
  WeightedSum cost(const std::vector<Arc::Index>& subset, Float pos) const;

  // node attributes
  bool persistent(Node::Index i) const { return node[i].parent != Node::null; }
  bool placed(Node::Index i) const { return node[i].pos >= Float(0); }

  Functional* functional;        // ordering functional
  Progress* progress;            // progress callbacks
  std::vector<Node::Index> perm; // ordered list of indices to nodes
  std::vector<Node> node;        // statically ordered list of nodes
  std::vector<Node::Index> adj;  // statically ordered list of adjacent nodes
  std::vector<Float> weight;     // statically ordered list of arc weights
  std::vector<Float> bond;       // statically ordered list of coarsening weights

private:
  // initialize graph with given number of nodes
  void init(uint nodes);

  // find optimal position of node i while fixing all other nodes
  Float optimal(Node::Index i) const;

  // add contribution of fine arc to coarse graph
  void update(Node::Index i, Node::Index j, Float w, Float b);

  // transfer contribution of fine arc a to coarse node p
  void transfer(Graph* g, const std::vector<Float>& part, Node::Index p, Arc::Index a, Float f = 1) const;

  // swap the positions of nodes
  void swap(uint k, uint l);

  // random number generator
  static uint random(uint seed = 0);

  uint level;            // level of coarsening
  Node::Index last_node; // last node with outgoing arcs
};

}

#endif
