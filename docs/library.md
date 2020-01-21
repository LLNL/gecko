API
===

The interface to the library is quite straightforward, and is
contained entirely in the public section of the `Gecko::Graph` class
(see `include/graph.h`).  Nevertheless, be sure to read this whole
section as it is **important to understand the assumptions and restrictions
gecko imposes**.

Graph Construction
------------------

A graph is first created by inserting nodes and edges via

    Graph::insert_node();
    Graph::insert_arc(Node::Index i, Node::Index j, Float weight = 1);

An undirected edge {*i*, *j*} is represented as two directed edges,
called *arcs*, (*i*, *j*) and (*j*, *i*).  Although the API only supports
directed edges, the final graph must be undirected and contain both (*i*, *j*)
and (*j*, *i*).  It is up to the user to ensure this consistency.

Edge weights are optional and default to one.  Node and arc indices start
at one, e.g., 1 <= *i*, *j*, <= *V*, where *V* is the number of nodes in the
graph.  Node and arc index 0 represents null.  No self loops {*i*, *i*} are
allowed.

`Graph::insert_node()` returns the index associated with the new node.  Such
node indices are guaranteed to be consecutive and start at 1 (not 0).
`Graph::insert_arc(i, j)` requires that the source node *i* is a valid node
previously created by `Graph::insert_node()`.  However, the target node *j*
needs not exist in the graph at the time of arc insertion.

### IMPORTANT!!!

Arcs (*i*, *j*) must be inserted in increasing order of the source node *i*.
Thus, graph construction should insert one node at a time.  With each such
node *i*, all outgoing arcs (*i*, *j*) from *i* should then be inserted.
Incoming arcs will be inserted with each neighbor *j* of *i*.  We recognize
that this restriction on arc insertions is inconvenient, and we hope to
lift it in future versions of gecko.  If an arc is inserted out of order,
`Graph::insert_arc()` will return `Arc::null`.


Graph Ordering
--------------

A single call is then made to order the graph using `Graph::order()`.
This function takes a number of parameters that govern the ordering
algorithm.  See the document `docs/algorithm.md` for a description of these
parameters.  The `Graph::order()` function computes a permutation in
each V-cycle and stores the best permutation found.  This permutation
can then be queried using the functions

    const std::vector<Node::Index>& Graph::permutation() const;
    Node::Index Graph::permutation(uint rank) const;
    uint Graph::rank(Node::Index i) const;

The permutation vector gives the nodes in the order they appear in the
optimized layout.  Alternatively, the final position of each node may be
requested using `Graph::rank()`.

### Progress Reporting

Graph ordering can be a lengthy process depending on graph size and algorithm
parameter settings.  Gecko provides a callback mechanism for reporting where
it is in the layout process, which may be communicated to the user.
Moreover, it is possible to terminate the ordering process permaturely, e.g.,
via a signal handler that catches CTRL-C and sets a Boolean that is
periodically checked by gecko.  For an example of how to use progress
reporting, see the gecko command-line utility `utils/gecko.cpp`.
