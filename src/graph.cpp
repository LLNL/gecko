#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "gecko/graph.h"
#include "subgraph.h"
#include "heap.h"

using namespace std;
using namespace Gecko;

// Constructor.
void
Graph::init(uint nodes)
{
  node.push_back(Node(-1, 0, 1, Node::null));
  adj.push_back(Node::null);
  weight.push_back(0);
  bond.push_back(0);
  while (nodes--)
    insert_node();
}

// Insert node.
Node::Index
Graph::insert_node(Float length)
{
  Node::Index p = Node::Index(node.size());
  perm.push_back(p);
  node.push_back(Node(-1, length));
  return p;
}

// Return nodes adjacent to i.
std::vector<Node::Index>
Graph::node_neighbors(Node::Index i) const
{
  std::vector<Node::Index> neighbor;
  for (Arc::Index a = node_begin(i); a < node_end(i); a++)
    neighbor.push_back(adj[a]);
  return neighbor;
}

// Insert directed edge (i, j).
Arc::Index
Graph::insert_arc(Node::Index i, Node::Index j, Float w, Float b)
{
  if (!i || !j || i == j || !(last_node <= i && i <= nodes()))
    return Arc::null;
  last_node = i;
  for (Node::Index k = i - 1; node[k].arc == Arc::null; k--)
    node[k].arc = Arc::Index(adj.size());
  adj.push_back(j);
  weight.push_back(w);
  bond.push_back(b);
  node[i].arc = Arc::Index(adj.size());
  return Arc::Index(adj.size() - 1);
}

// Remove arc a.
bool
Graph::remove_arc(Arc::Index a)
{
  if (a == Arc::null)
    return false;
  Node::Index i = arc_source(a);
  adj.erase(adj.begin() + a);
  weight.erase(weight.begin() + a);
  bond.erase(bond.begin() + a);
  for (Node::Index k = i; k < node.size(); k++)
    node[k].arc--;
  return true;
}

// Remove directed edge (i, j).
bool
Graph::remove_arc(Node::Index i, Node::Index j)
{
  return remove_arc(arc_index(i, j));
}

// Remove edge {i, j}.
bool
Graph::remove_edge(Node::Index i, Node::Index j)
{
  bool success = remove_arc(i, j);
  if (success)
    success = remove_arc(j, i);
  return success;
}

// Index of arc (i, j) or null if not a valid arc.
Arc::Index
Graph::arc_index(Node::Index i, Node::Index j) const
{
  for (Arc::Index a = node_begin(i); a < node_end(i); a++)
    if (adj[a] == j)
      return a;
  return Arc::null;
}

// Return source node i in arc a = (i, j).
Node::Index
Graph::arc_source(Arc::Index a) const
{
  Node::Index j = adj[a];
  for (Arc::Index b = node_begin(j); b < node_end(j); b++) {
    Node::Index i = adj[b];
    if (node_begin(i) <= a && a < node_end(i))
      return i;
  }
  // should never get here
  throw std::runtime_error("internal data structure corrupted");
}

// Return reverse arc (j, i) of arc a = (i, j).
Arc::Index
Graph::reverse_arc(Arc::Index a) const
{
  Node::Index j = adj[a];
  for (Arc::Index b = node_begin(j); b < node_end(j); b++) {
    Node::Index i = adj[b];
    if (node_begin(i) <= a && a < node_end(i))
      return b;
  }
  return Arc::null;
}

// Return first directed arc if one exists or null otherwise.
Arc::Index
Graph::directed() const
{
  for (Node::Index i = 1; i < node.size(); i++)
    for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
      Node::Index j = adj[a];
      if (!arc_index(j, i))
        return a;
    }
  return Arc::null;
}

// Add contribution of fine arc to coarse graph.
void
Graph::update(Node::Index i, Node::Index j, Float w, Float b)
{
  Arc::Index a = arc_index(i, j);
  if (a == Arc::null)
    insert_arc(i, j, w, b);
  else {
    weight[a] += w;
    bond[a] += b;
  }
}

// Transfer contribution of fine arc a to coarse node p.
void
Graph::transfer(Graph* g, const vector<Float>& part, Node::Index p, Arc::Index a, Float f) const
{
  Float w = f * weight[a];
  Float m = f * bond[a];
  Node::Index j = arc_target(a);
  Node::Index q = node[j].parent;
  if (q == Node::null) {
    for (Arc::Index b = node_begin(j); b < node_end(j); b++)
      if (part[b] > 0) {
        q = node[adj[b]].parent;
        if (q != p)
          g->update(p, q, w * part[b], m * part[b]);
      }
  }
  else
    g->update(p, q, w, m);
}

// Compute cost of a subset of arcs incident on node placed at pos.
WeightedSum
Graph::cost(const vector<Arc::Index>& subset, Float pos) const
{
  WeightedSum c;
  for (Arc::ConstPtr ap = subset.begin(); ap != subset.end(); ap++) {
    Arc::Index a = *ap;
    Node::Index j = arc_target(a);
    Float l = fabs(node[j].pos - pos);
    Float w = weight[a];
    functional->accumulate(c, WeightedValue(l, w));
  }
  return c;
}

// Compute cost of graph layout.
Float
Graph::cost() const
{
  if (edges()) {
    WeightedSum c;
    Node::Index i = 1;
    for (Arc::Index a = 1; a < adj.size(); a++) {
      while (node_end(i) <= a)
        i++;
      Node::Index j = arc_target(a);
      Float l = length(i, j);
      Float w = weight[a];
      functional->accumulate(c, WeightedValue(l, w));
    }
    return functional->mean(c);
  }
  else
    return Float(0);
}

// Swap the two nodes in positions k and l, k <= l.
void
Graph::swap(uint k, uint l)
{
  Node::Index i = perm[k];
  perm[k] = perm[l];
  perm[l] = i;
  Float p = node[i].pos - node[i].hlen;
  do {
    i = perm[k];
    p += node[i].hlen;
    node[i].pos = p;
    p += node[i].hlen;
  } while (k++ != l);
}

// Optimize continuous position of a single node.
Float
Graph::optimal(Node::Index i) const
{
  vector<WeightedValue> v;
  for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
    Node::Index j = adj[a];
    if (placed(j))
      v.push_back(WeightedValue(node[j].pos, weight[a]));
  }
  return v.empty() ? -1 : functional->optimum(v);
}

// Compute coarse graph with roughly half the number of nodes.
Graph*
Graph::coarsen()
{
  progress->beginphase(this, string("coarse"));
  Graph* g = new Graph(0, level - 1);
  g->functional = functional;
  g->progress = progress;

  // Compute importance of nodes in fine graph.
  DynamicHeap<Node::Index, Float> heap;
  for (Node::Index i = 1; i < node.size(); i++) {
    node[i].parent = Node::null;
    Float w = 0;
    for (Arc::Index a = node_begin(i); a < node_end(i); a++)
      w += bond[a];
    heap.insert(i, w);
  }

  // Select set of important nodes from fine graph that will remain in
  // coarse graph.
  vector<Node::Index> child(1, Node::null);
  while (!heap.empty()) {
    Node::Index i;
    Float w = 0;
    heap.extract(i, w);
    if (w < 0)
      break;
    child.push_back(i);
    node[i].parent = g->insert_node(2 * node[i].hlen);

    // Reduce importance of neighbors.
    for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
      Node::Index j = adj[a];
      if (heap.find(j, w))
        heap.update(j, w - 2 * bond[a]);
    }
  }

  // Assign parts of remaining nodes to aggregates.
  vector<Float> part = bond;
  for (Node::Index i = 1; i < node.size(); i++)
    if (!persistent(i)) {
      // Find all connections to coarse nodes.
      Float w = 0;
      Float max = 0;
      for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
        Node::Index j = adj[a];
        if (persistent(j)) {
          w += part[a];
          if (max < part[a])
            max = part[a];
        }
        else
          part[a] = -1;
      }
      max /= GECKO_PART_FRAC;

      // Weed out insignificant connections.
      for (Arc::Index a = node_begin(i); a < node_end(i); a++)
        if (0 < part[a] && part[a] < max) {
          w -= part[a];
          part[a] = -1;
        }

      // Compute node fractions (interpolation matrix) and assign
      // partial nodes to aggregates.
      for (Arc::Index a = node_begin(i); a < node_end(i); a++)
        if (part[a] > 0) {
          part[a] /= w;
          Node::Index p = node[adj[a]].parent;
          g->node[p].hlen += part[a] * node[i].hlen;
        }
    }

  // Transfer arcs to coarse graph.
  for (Node::Index p = 1; p < g->node.size(); p++) {
    Node::Index i = child[p];
    for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
      transfer(g, part, p, a);
      Node::Index j = adj[a];
      if (!persistent(j)) {
        Arc::Index b = arc_index(j, i);
        if (part[b] > 0)
          for (Arc::Index c = node_begin(j); c < node_end(j); c++) {
            Node::Index k = adj[c];
            if (k != i)
              transfer(g, part, p, c, part[b]);
          }
      }
    }
  }

#if DEBUG
  if (g->directed())
    throw runtime_error("directed edge found");
#endif

  // Free memory.
  vector<Float> t = bond;
  bond.swap(t);

  progress->endphase(this, false);

  return g;
}

// Order nodes according to coarsened graph layout.
void
Graph::refine(const Graph* graph)
{
  progress->beginphase(this, string("refine"));

  // Place persistent nodes.
  DynamicHeap<Node::Index, Float> heap;
  for (Node::Index i = 1; i < node.size(); i++)
    if (persistent(i)) {
      Node::Index p = node[i].parent;
      node[i].pos = graph->node[p].pos;
    }
    else {
      node[i].pos = -1;
      Float w = 0;
      for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
        Node::Index j = adj[a];
        if (persistent(j))
          w += weight[a];
      }
      heap.insert(i, w);
    }

  // Place remaining nodes in order of decreasing connectivity with
  // already placed nodes.
  while (!heap.empty()) {
    Node::Index i = 0;
    heap.extract(i);
    node[i].pos = optimal(i);
    for (Arc::Index a = node_begin(i); a < node_end(i); a++) {
      Node::Index j = adj[a];
      Float w;
      if (heap.find(j, w))
        heap.update(j, w + weight[a]);
    }
  }

  place(true);
  progress->endphase(this, true);
}

// Perform m sweeps of compatible or Gauss-Seidel relaxation.
void
Graph::relax(bool compatible, uint m)
{
  progress->beginphase(this, compatible ? string("crelax") : string("frelax"));
  while (m--)
    for (uint k = 0; k < perm.size() && !progress->quit(); k++) {
      Node::Index i = perm[k];
      if (!compatible || !persistent(i))
        node[i].pos = optimal(i);
    }
  place(true);
  progress->endphase(this, true);
}

// Optimize successive n-node subgraphs.
void
Graph::optimize(uint n)
{
  if (n > perm.size())
    n = uint(perm.size());
  ostringstream count;
  count << setw(2) << n;
  progress->beginphase(this, string("perm") + count.str());
  Subgraph* subgraph = new Subgraph(this, n);
  for (uint k = 0; k <= perm.size() - n && !progress->quit(); k++)
    subgraph->optimize(k);
  delete subgraph;
  progress->endphase(this, true);
}

// Place all nodes according to their positions.
void
Graph::place(bool sort)
{
  place(sort, 0, uint(perm.size()));
}

// Place nodes {k, ..., k + n - 1} according to their positions.
void
Graph::place(bool sort, uint k, uint n)
{
  // Place nodes.
  if (sort)
    stable_sort(perm.begin() + k, perm.begin() + k + n, Node::Comparator(node.begin()));

  // Assign node positions according to permutation.
  for (Float p = k ? node[perm[k - 1]].pos + node[perm[k - 1]].hlen : 0; n--; k++) {
    Node::Index i = perm[k];
    p += node[i].hlen;
    node[i].pos = p;
    p += node[i].hlen;
  }
}

// Perform one V-cycle.
void
Graph::vcycle(uint n, uint work)
{
  if (n < nodes() && nodes() < edges() && level && !progress->quit()) {
    Graph* graph = coarsen();
    graph->vcycle(n, work + edges());
    refine(graph);
    delete graph;
  }
  else
    place();
  if (edges()) {
    relax(true, GECKO_CR_SWEEPS);
    relax(false, GECKO_GS_SWEEPS);
    for (uint w = edges(); w * (n + 1) < work; w *= ++n);
    n = std::min(n, uint(GECKO_WINDOW_MAX));
    if (n)
      optimize(n);
  }
}

// Custom random-number generator for reproducibility.
// LCG from doi:10.1090/S0025-5718-99-00996-5.
uint
Graph::random(uint seed)
{
  static uint state = 1;
  state = (seed ? seed : 0x1ed0675 * state + 0xa14f);
  return state;
}

// Generate a random permutation of the nodes.
void
Graph::shuffle(uint seed)
{
  random(seed);
  for (uint k = 0; k < perm.size(); k++) {
    uint r = random() >> 8;
    uint l = k + r % (uint(perm.size()) - k);
    std::swap(perm[k], perm[l]);
  }
  place();
}

// Recompute bonds for k'th V-cycle.
void
Graph::reweight(uint k)
{
  bond.resize(weight.size());
  for (Arc::Index a = 1; a < adj.size(); a++)
    bond[a] = functional->bond(weight[a], length(a), k);
}

// Linearly order graph.
void
Graph::order(Functional* functional, uint iterations, uint window, uint period, uint seed, Progress* progress)
{
  // Initialize graph.
  this->functional = functional;
  progress = this->progress = progress ? progress : new Progress;
  for (level = 0; (1u << level) < nodes(); level++);
  place();
  Float mincost = cost();
  vector<Node::Index> minperm = perm;
  if (seed)
    shuffle(seed);

  progress->beginorder(this, mincost);
  if (edges()) {
    // Perform specified number of V-cycles.
    for (uint k = 1; k <= iterations && !progress->quit(); k++) {
      progress->beginiter(this, k, iterations, window);
      reweight(k);
      vcycle(window);
      Float c = cost();
      if (c < mincost) { 
        mincost = c;
        minperm = perm;
      }
      progress->enditer(this, mincost, c);
      if (period && !(k % period))
        window++;
    }
    perm = minperm;
    place();
  }
  progress->endorder(this, mincost);

  if (!progress) {
    delete this->progress;
    this->progress = 0;
  }
}
