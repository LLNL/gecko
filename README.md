Gecko
=====

Gecko is a C++ library for solving graph linear arrangement problems.  Gecko
orders graph nodes, representing data elements, connected by undirected edges,
representing affinity relations, with the goal of minimizing a chosen
functional of edge length.  Gecko was primarily designed to minimize the
product of edge lengths but can also be used to reduce bandwidth (maximum
edge length) and 1-sum (sum of edge lengths), among others.
Minimum-edge-product orderings generalize space-filling curve orderings to
geometryless graphs and find applications in data locality optimization,
graph partitioning, and dimensionality reduction.


Quick Start
-----------

Detailed installation instructions are available in `docs/installation.md`.
Below are basic instructions for building gecko.

To download the latest version of the source code, type:

    git clone https://github.com/LLNL/gecko.git

Gecko supports both CMake and GNU builds.

### CMake builds

From a Unix or git shell, type:

    cd gecko
    mkdir build
    cd build
    cmake ..
    cmake --build . --config Release

This builds the gecko library in `build/lib` and executable in `build/bin`.
To display the compile-time options, type `cmake .. -L`.

### GNU builds

Gecko can also be built using GNU make:

    cd gecko
    make

This builds the gecko library and executable in `lib` and `bin`.  For
compile-time options, please consult the file `Config`.


Author
------

Gecko was written by [Peter Lindstrom](https://people.llnl.gov/pl) at
Lawrence Livermore National Laboratory.


Citing Gecko
------------

If you use gecko for scholarly research, please cite the following paper:

* Peter Lindstrom
  [The Minimum Edge Product Linear Ordering Problem](https://www.researchgate.net/publication/259383744_The_Minimum_Edge_Product_Linear_Ordering_Problem)
  LLNL technical report LLNL-TR-496076, August 26, 2011.


Support
-------

Please submit bug reports and feature requests via the GitHub
[issue tracker](https://github.com/LLNL/gecko/issues).
For all other questions and comments, please contact us at
[gecko@llnl.gov](mailto:gecko@llnl.gov).


License
-------

Gecko is distributed under the terms of the BSD license.  See the files
[LICENSE](https://github.com/LLNL/gecko/blob/master/LICENSE) and
[NOTICE](https://github.com/LLNL/gecko/blob/master/NOTICE) for details.

SPDX-License-Identifier: BSD

LLNL-CODE-800597
