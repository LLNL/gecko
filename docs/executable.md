Executable
==========

The command-line utility `bin/gecko` orders graphs stored in the text format
read by
[Chaco](https://cfwebprod.sandia.gov/cfdocs/CompResearch/docs/guide.pdf) and
[Metis](http://glaros.dtc.umn.edu/gkhome/views/metis/).
Such files begin with a line listing the number of vertices, *V*, and edges,
*E*, which is followed by *V* lines that for each node, 1 <= *i* <= *V*,
lists the indices of all nodes adjacent to node *i*.  Example graphs are
available in the `data` directory.  The input graph is read from standard
input, and the reordered position, 0 <= *p* <= *V* - 1, of each node is
written to standard output.

The graph may be weighted to emphasize the relative importance that adjacent
nodes be placed together (larger weights imply higher importance).  Weighted
graphs have a third entry, `1`, on the first line of input and a positive
real number following each neighbor index to encode the weight of the
edge between the node and its neighbor.

A number of command-line options are provided that greatly control the
quality of the layout and the running time.  The usage is:

    order <functional> [iterations [window [period [seed [psfile]]]]]

See the document `docs/algorithm.md` for a description of these parameters.

A reasonable parameter choice for good-quality layouts of medium-sized
graphs (with, say, 100,000 nodes) is iterations = 4, window = 4, period = 2.
As the window gets progressively larger, the running time of each iteration
will rapidly increase.  For this reason, the executable can be terminated
early via CTRL-C, at which point the best layout found so far is output.
The value of the objective function `f` and the lowest `f` found so far are
output at the end of each cycle to provide feedback on progress.  The `f`
value of the input layout is also output before the first iteration.
