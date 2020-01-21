#include <algorithm>
#include "gecko/drawing.h"

using namespace std;

#define SOURCE_BOT 1
#define SOURCE_MID 2
#define SOURCE_TOP 3
#define TARGET_BOT 4
#define TARGET_MID 5
#define TARGET_TOP 6

namespace Gecko {

// node record for sorting
class NodeRef {
public:
  NodeRef(Float x, Node::Index i) : pos(x), index(i) {}
  bool operator<(const NodeRef& node) const { return pos < node.pos; }
  Float pos;
  Node::Index index;
};

// arc record for sorting
class ArcRef {
public:
  ArcRef(Float l, Arc::Index a) : length(l), index(a) {}
  bool operator<(const ArcRef& arc) const { return length < arc.length; }
  Float length;
  Arc::Index index;
};

// draw graph
void
Drawing::draw(const Graph* g)
{
  device->begin();

  // sort nodes by increasing position, arcs by increasing length
  vector<NodeRef> node;
  vector<ArcRef> arc;
  for (Node::Index i = 1; i <= g->nodes(); i++) {
    node.push_back(NodeRef(g->node[i].pos, i));
    for (Arc::Index a = g->node_begin(i); a < g->node_end(i); a++) {
      Node::Index j = g->arc_target(a);
      if (g->node[i].pos < g->node[j].pos)
        arc.push_back(ArcRef(g->length(i, j), a));
    }
  }
  sort(node.begin(), node.end());
  sort(arc.begin(), arc.end());

  // assign arc attachment points
  vector<unsigned char> anchor(2 * g->edges() + 1, 0);
  for (vector<ArcRef>::const_iterator p = arc.begin(); p != arc.end(); p++)
    if (p->length >= 0.5) {
      Arc::Index a = p->index;
      Arc::Index b = g->reverse_arc(a);
      Node::Index i = g->arc_source(a);
      Node::Index j = g->arc_target(a);
      uint source[7] = {};
      uint target[7] = {};
      for (Arc::Index c = g->node_begin(i); c < g->node_end(i); c++)
        source[anchor[c]]++;
      for (Arc::Index c = g->node_begin(j); c < g->node_end(j); c++)
        target[anchor[c]]++;
      if (!source[SOURCE_MID] && !target[TARGET_MID]) {
        vector<NodeRef>::const_iterator q = upper_bound(node.begin(), node.end(), NodeRef(g->node[i].pos, i));
        if (q->pos >= g->node[j].pos) {
          // no nodes are between i and j
          anchor[a] = SOURCE_MID;
          anchor[b] = TARGET_MID;
          continue;
        }
      }
      if (source[SOURCE_BOT] + target[TARGET_BOT] < source[SOURCE_TOP] + target[TARGET_TOP]) {
        anchor[a] = SOURCE_BOT;
        anchor[b] = TARGET_BOT;
      }
      else {
        anchor[a] = SOURCE_TOP;
        anchor[b] = TARGET_TOP;
      }
    }

  // draw arcs
  for (vector<ArcRef>::const_reverse_iterator p = arc.rbegin(); p != arc.rend(); p++) {
    Arc::Index a = p->index;
    Node::Index i = g->arc_source(a);
    Node::Index j = g->arc_target(a);
    switch (anchor[a]) {
      case SOURCE_BOT:
      case TARGET_BOT:
        device->edge(g->node[i].pos, g->node[j].pos, g->weight[a], false);
        break;
      case SOURCE_TOP:
      case TARGET_TOP:
        device->edge(g->node[i].pos, g->node[j].pos, g->weight[a], true);
        break;
      case SOURCE_MID:
      case TARGET_MID:
        device->edge(g->node[i].pos, g->node[j].pos, g->weight[a]);
        break;
    }
  }

  // draw nodes
  for (Node::Index i = 1; i <= g->nodes(); i++)
    device->node(g->node[i].pos, Float(0.5) * g->node[i].hlen, g->persistent(i) ? Float(0.25) : Float(0.75));

  device->end();
}

}
