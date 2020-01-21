Frequently Asked Questions
==========================

For questions not answered here, please contact us at
[gecko@llnl.gov](mailto:gecko@llnl.gov).


## Why does arc insertion sometimes fail?

First, when inserting arc (*i*, *j*), the graph must have at least *i* nodes.
Second, the order of arcs being inserted must be sorted on the source node,
*i* (but not on the target node, *j*).  If either condition is violated, then
`null` is returned.  This requirement exists for legacy reasons and to ensure
fast construction of the graph data structures.


## How should I set the ordering parameters?

We suggest by starting with a single iteration (V-cycle) and a window of four
nodes.  This is a good compromise between ordering quality and speed, and is
appropriate for applications that require ordering at runtime.
For ordering problems that need to be solved only once, we suggest using
multiple iterations.  For instance, a handful of iterations with a period
of one or two.  See `docs/algorithm.md` for a description of the parameters.


## Should I consider multiple invocations of gecko?

Gecko attempts to solve an optimization problem that usually has many local
minima.  Therefore it is possible that any given execution will result in
a suboptimal solution.  To explore the space of possible orderings, we
recommend executing gecko many times--when practical--with different random
seeds and then keeping the best layout found, as determined by calling the
`Graph::cost()` function.

Gecko randomly shuffles the initial layout and takes as input a random seed,
which ensures reproducibility.  This randomization initializes the optimization
algorithm and steers it in a particular "direction" of the search space to
be explored.  When execution time is at a premium, such as in runtime
ordering, a single run may suffice.  When ordering is done offline, we
recommend exploring a dozen or more random seeds.


## How does gecko scale to large graphs?

Gecko works best for graphs with a few tens of thousands of nodes, but has
been used to order graphs up to a few million nodes.  For such large graphs,
we recommend using only a single iteration and a small optimization window,
such as 2-4 nodes.
