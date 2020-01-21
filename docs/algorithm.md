Algorithm
=========

Gecko orders the nodes of an undirected and optionally weighted graph in an
effort to place nodes connected by an edge in consecutive positions in the
linear ordering (aka. *layout*).  Such orderings promote good data locality,
e.g., to improve cache utilization, but also find applications in graph
partitioning and dimensionality reduction.

The gecko ordering method is inspired by algebraic multigrid methods, and
uses V-cycles to coarsen, optimize, and refine the graph layout.
The graph constitutes an abstract representation of the relationship
between elements in a data set, e.g., a graph node may represent a
vertex or a cell in a mesh, a pixel in an image, a node in a binary
search tree, an element in a sparse matrix, etc.  The graph edges
represent node affinities, or a desire that adjacent nodes be stored
close together on linear storage (e.g., disk or main memory).  Such a
data layout is likely to improve cache utilization in block-based
caches common on today's computer architectures.  For instance, the
edges may connect adjacent pixels in an image, as many image
processing operations involve accessing local neighborhoods.  The
resulting node layouts are "cache-oblivious" in the sense that no
particular knowledge of the cache parameters (number and size of
blocks, associativity, replacement policy, etc.) are accounted for.
Rather, the expectation is that the layouts will provide good
locality across all levels of cache.  Note that the ordering method
accepts any undirected graph, whether it represent a structured or
unstructured data set, and is also oblivious of any geometric
structure inherent in the data set.

The optimization algorithm attempts to order the nodes of the graph
so as to minimize the geometric mean edge length, or equivalently
the product

    product |p(i) - p(j)|^w(i, j)

or weighted sum

    sum w(i, j) log(|p(i) - p(j)|)

where *i* and *j* are nodes joined by an edge, *w*(*i*, *j*) is a positive
edge weight (equal to one unless otherwise specified), *p*(*i*) is
the integer position of node *i* in the linear layout of the graph
(with *p*(*i*) = *p*(*j*) if and only if *i* = *j*), and where the product
or sum is over all edges of the graph.

The algorithm is described in further detail in the paper

* Peter Lindstrom
  [The Minimum Edge Product Linear Ordering Problem](https://www.researchgate.net/publication/259383744_The_Minimum_Edge_Product_Linear_Ordering_Problem)
  LLNL technical report LLNL-TR-496076, August 26, 2011.


Ordering Parameters
-------------------

The `Graph::order()` function and the `gecko` command-line executable take a
number of parameters that govern the layout process.  These parameters are
described below:

* The **functional** is the objective being optimized and expresses the cost
  of the graph layout in terms of some average of its edge lengths
  |*p*(*i*) - *p*(*j*)|.  The predefined functionals are
  * `h` (harmonic mean)
  * `g` (geometric mean)
  * `s` (square mean root)
  * `a` (arithmetic mean)
  * `r` (root mean square)
  * `m` (maximum)

  Note that the algorithm has not been well tuned or tested to optimize
  functionals other than the geometric mean.

* The number of **iterations** specifies the number of multigrid V-cycles
  to perform.  Usually a handful of cycles is sufficient.  The default is
  a single cycle.

* The optimization **window** is the number of consecutive nodes optimized
  concurrently using exhaustive search.  The larger the window, the higher
  the quality.  Note that the running time increases exponentially with the
  window size.  Usually a window no larger than six nodes is sufficient.
  The default is a window size of two nodes.

* The **period** is the number of V-cycles to run between increments of the
  window size.  Usually it is beneficial to start with a small window to get
  a rough layout, and to then increase the window size to fine-tune the
  layout.  The default is a period of one cycle.

* The random **seed** allows injecting some randomness in the optimization
  process.  When the seed is nonzero, the nodes are randomly shuffled prior
  to invoking the ordering algorithm, thereby affecting subsequent coarsening
  and ordering decisions.  In effect, this randomization allows different
  directions to be explored in the combinatorial optimization space.  Fixing
  the seed allows for reproducibility, i.e., the same seed always leads to
  the same layout.  Since the global optimum is seldom (if ever) reached,
  it often makes sense to run several instances of the algorithm, each with
  a new random seed, and to pick the best layout found.  In the gecko
  executable, the current time is used as random seed if not specified.

* The **psfile** parameter (executable only) is the name of an optional
  PostScript file that shows the linear node order on each level of
  refinement.  This capability is useful for debugging the ordering of
  small graphs.

A reasonable parameter choice for good-quality layouts is:

* iterations = 4
* window = 4
* period = 2
