#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "gecko.h"
#include "gecko/graph.h"

using namespace Gecko;

template <typename T>
static std::string
stringize(T val)
{
  std::ostringstream s;
  s << val;
  return s.str();
}

// print library version info
static bool
init()
{
  std::cerr << Gecko::version_string << std::endl;
  std::cerr << "library version " << std::showbase << std::hex << Gecko::version << std::dec << std::endl;
  std::cerr << std::endl;
  return Gecko::version == GECKO_VERSION;
}

// summarize tests and return exit code
static int
finish(size_t failures, size_t tests)
{
  std::cerr << std::endl;
  if (failures)
    std::cerr << failures << " test" << (failures > 1 ? "s" : "") << " of " << tests << " failed" << std::endl;
  else
    std::cerr << "all tests passed" << std::endl;
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

// order linear path of given number of nodes and ensure Hamiltonicity
static std::string
path_test(
  uint nodes = 0x10000, // number of nodes
  uint iterations = 1,  // number of V cycles
  uint window = 4,      // initial window size
  uint period = 0,      // iterations between window increment
  uint seed = 1         // random number seed
)
{
  uint edges = nodes - 1;

  // construct graph
  Graph graph(nodes);

  // insert arcs
  for (Node::Index i = 1; i <= nodes; i++) {
    if (i > 1)
      graph.insert_arc(i, i - 1);
    if (i < nodes)
      graph.insert_arc(i, i + 1);
  }
  if (graph.nodes() != nodes)
    return std::string("incorrect node count");
  if (graph.edges() != edges)
    return std::string("incorrect edge count");

  // order graph
  Functional* functional = new FunctionalGeometric();
  graph.order(functional, iterations, window, period, seed);
  delete functional;

  // ensure that consecutive nodes are connected by an edge
  for (uint rank = 0; rank < nodes - 1; rank++) {
    Node::Index i = graph.permutation(rank + 0);
    Node::Index j = graph.permutation(rank + 1);
    if (!graph.arc_index(i, j))
      return std::string("{") + stringize(i) + std::string(", ") + stringize(j) + std::string("} is not an edge");
  }

  return std::string();
}

// order hypercube of given number of dimensions and ensure Hamiltonicity
static std::string
hypercube_test(
  uint dims,           // number of hypercube dimensions
  uint iterations = 4, // number of V cycles
  uint window = 6,     // initial window size
  uint period = 1,     // iterations between window increment
  uint seed = 1        // random number seed
)
{
  uint nodes = 1u << dims;       // hypercube node count
  uint edges = dims * nodes / 2; // hypercube edge count; one arc per node and dimension

  // construct graph
  Graph graph(nodes);

  // insert edges (both (i, j) and (j, i) are needed)
  for (Node::Index i = 1; i <= nodes; i++)
    for (uint d = 0; d < dims; d++) {
      // insert arc along each hypercube dimension
      Node::Index j = ((i - 1) ^ (1u << d)) + 1;
      if (!graph.insert_arc(i, j))
        return std::string("arc insertion failed");
    }
  if (graph.nodes() != nodes)
    return std::string("incorrect node count");
  if (graph.edges() != edges)
    return std::string("incorrect edge count");

  // order graph
  Functional* functional = new FunctionalGeometric();
  graph.order(functional, iterations, window, period, seed);
  delete functional;

  // ensure that consecutive nodes are connected by an edge
  for (uint rank = 0; rank < nodes - 1; rank++) {
    Node::Index i = graph.permutation(rank + 0);
    Node::Index j = graph.permutation(rank + 1);
    if (!graph.arc_index(i, j))
      return std::string("{") + stringize(i) + std::string(", ") + stringize(j) + std::string("} is not an edge");
  }

  return std::string();
}

// order 2D grid of given width and ensure cost is minimal
static std::string
grid_test(
  uint size,           // number of nodes along each dimension
  uint iterations = 5, // number of V cycles
  uint window = 5,     // initial window size
  uint period = 1,     // iterations between window increment
  uint seed = 1        // random number seed
)
{
  // known minimal edge products
  double minproduct[] = { 0., 1., 3., 225., 688905., 145904338125., 984582541613671875. };

  if (size > sizeof(minproduct) / sizeof(minproduct[0]))
    return std::string("grid size exceeds maximum size");

  uint nodes = size * size;           // grid node count
  uint edges = 2 * size * (size - 1); // grid edge count

  Float mincost = edges ? Float(std::exp(std::log(minproduct[size]) / edges)) : Float(0);

  // construct graph
  Graph graph;

  // insert nodes
  for (Node::Index i = 1; i <= nodes; i++) {
    graph.insert_node();
    uint x = (i - 1) % size;
    uint y = (i - 1) / size;
    if (x > 0)
      graph.insert_arc(i, i - 1);
    if (x < size - 1)
      graph.insert_arc(i, i + 1);
    if (y > 0)
      graph.insert_arc(i, i - size);
    if (y < size - 1)
      graph.insert_arc(i, i + size);
  }
  if (graph.nodes() != nodes)
    return std::string("incorrect node count");
  if (graph.edges() != edges)
    return std::string("incorrect edge count");

  // order graph
  Functional* functional = new FunctionalGeometric();
  graph.order(functional, iterations, window, period, seed);
  Float cost = graph.cost();
  delete functional;

  Float epsilon = Float(1e-6);
  if (cost <= Float(1 + epsilon) * mincost)
    return std::string();
  else
    return stringize(cost) + " > " + stringize(mincost);
}

// report the result of a test and return 1 if it failed
static int
report(std::string test, std::string error, int columns = 20)
{
  std::cerr << std::setw(columns) << std::left << test << " ";
  if (error.empty()) {
    std::cerr << "[ OK ]" << std::endl;
    return 0;
  }
  else {
    std::cerr << "[FAIL] " << error << std::endl;
    return 1;
  }
}

int main(int argc, char* argv[])
{
  uint tests = 0;    // number of tests performed
  uint failures = 0; // number of failed tests
  uint maxdims = 5;  // number of hypercube dimensions
  uint maxsize = 6;  // max grid dimensions

  switch (argc) {
    case 3:
      if (sscanf(argv[2], "%u", &maxsize) != 1)
        goto usage;
      // FALLTHROUGH
    case 2:
      if (sscanf(argv[1], "%u", &maxdims) != 1)
        goto usage;
      // FALLTHROUGH
    case 1:
      break;
    default:
    usage:
      std::cerr << "Usage: testgecko [maxdims [maxsize]]" << std::endl;
      return EXIT_FAILURE;
  }

  // print version info and ensure header matches binary
  failures += report("library version test", init() ? std::string() : std::string("header and binary version mismatch"));
  tests++;

  // order path
  std::string error = path_test();
  failures += report("path graph test", error);
  tests++;

  // order hypercubes
  for (uint dims = 1; dims <= maxdims; dims++) {
    std::string error = hypercube_test(dims);
    failures += report(std::string("hypercube test #") + stringize(dims), error);
    tests++;
  }

  // order grids
  for (uint size = 1; size <= maxsize; size++) {
    std::string error = grid_test(size);
    failures += report(std::string("grid test #") + stringize(size), error);
    tests++;
  }

  // summarize tests
  return finish(failures, tests);
}
