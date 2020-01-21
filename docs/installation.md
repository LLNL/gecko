Installation
============

The file `README.md` contains basic instructions on building gecko.
More detailed build instructions follow below.


Basic Build
-----------

For a basic build, with default options, execute one of the command sequences
below from a Unix or git shell.

### CMake

To build gecko using CMake, execute the following commands:

    cd gecko
    mkdir build
    cd build
    cmake .. [options]
    cmake --build . --config Release

Here `[options]` indicate optional arguments to CMake, as discussed below.

### GNU Make

Gecko can also be built using GNU make:

    cd gecko
    make [options]


Configuration
-------------

Build options for CMake and GNU make are shared.  For example, to build
gecko as a shared library using CMake, replace `cmake ..` above with

    cmake .. -DBUILD_ALL=ON

The equivalent GNU make line reads

    make BUILD_ALL=1

Alternatively, the GNU make configuration file `Config` may be edited,
although we recommend setting options on the command line.

CMake Boolean options are either `OFF` or `ON`, while the corresponding
GNU make options are `0` or `1`.

The following build options are available:

* `BUILD_ALL`: Build all available targets (default = off).
* `BUILD_TESTING`: Build regression tests (default = on).
* `BUILD_UTILITIES`: Build command-line utilities (default = on).
* `BUILD_SHARED_LIBS`: Build shared libraries rather than static libraries
  (default = on).

The following compile-time macros are available for configuring the gecko
algorithm (see also `src/options.h`).  Rarely should these macros be modified.

* `GECKO_CR_SWEEPS`: Number of compatible relaxation sweeps (default = 1).
* `GECKO_GS_SWEEPS`: Number of Gauss-Seidel relaxation sweeps (default = 1).
* `GECKO_PART_FRAC`: Ratio of maximum to minimum weight for aggregation
  (default = 4).
* `GECKO_WINDOW_MAX`: Maximum number of consecutive nodes to exhaustively
  optimize (default = 16).
* `GECKO_WITH_ADJLIST`: Use adjacency list instead of adjacency matrix
  (default = off).
* `GECKO_WITH_NONRECURSIVE`: Use nonrecursive permutation algorithm
  (default = off).
* `GECKO_WITH_DOUBLE_PRECISION`: Perform computations in double rather
  than single precision (default = off).


Regression Testing
------------------

Basic regression tests are provided to ensure that the library is
functioning properly.  To run the regression tests, make sure
`BUILD_TESTING` is enabled (the default).  Using CMake, run `ctest -V`
to execute the tests after the test have been built.  If building with
GNU make, type `make test` instead from the top-level directory.


Installation
------------

To install the headers and library using CMake, type `cmake install`.
The path to where headers and binaries are installed may be set
when first running CMake via `cmake .. -DCMAKE_INSTALL_PREFIX=path`,
where `path` is the path to the parent directory of `include` and
`lib`, e.g., `/usr/local`.
