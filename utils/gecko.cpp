#include <csignal>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "gecko.h"
#include "gecko/graph.h"
#include "gecko/drawing.h"
#include "gecko/postscript.h"

using namespace Gecko;

//@p-r-i-v-a-t-e---t-y-p-e-s---------------------------------------------------

// progress reporting
class MyProgress : public Progress {
public:
  MyProgress(Drawing* d = 0) : drawing(d) {}
  void beginorder(const Graph*, Float cost) const
  {
    std::cerr << "f = " << std::fixed << std::setprecision(6) << cost << " { " << now();
  }
  void endorder(const Graph* graph, Float cost) const
  {
    std::cerr << "f = " << std::fixed << std::setprecision(6) << cost << " } " << now();
    if (drawing)
      drawing->draw(graph);
  }
  void beginiter(const Graph*, unsigned int iter, unsigned int maxiter, unsigned int window) const
  {
    std::cerr << "i = " << std::setw(width(maxiter)) << iter << "/" << maxiter << "  n = " << window << "  ";
  }
  void enditer(const Graph*, Float mincost, Float cost) const
  {
    std::cerr << "f = " << std::fixed << std::setprecision(6) << cost << "/" << mincost << "  " << now();
  }
#if VERBOSE
  void beginphase(const Graph* graph, std::string name) const
  {
    std::cerr << name << " V=" << std::setw(6) << graph->nodes() << " E=" << std::setw(6) << graph->edges();
  }
#else
  void beginphase(const Graph*, std::string) const {}
#endif
  void endphase(const Graph* graph, bool show) const
  {
#if VERBOSE
    if (show)
      std::cerr << " f=" << fixed << setw(12) << std::setprecision(6) << graph->cost();
    std::cerr << std::endl;
#endif
    if (drawing && show)
      drawing->draw(graph);
  }
  bool quit() const { return _exit; }
  static void exit() { _exit = true; }

private:
  static volatile bool _exit;
  Drawing* drawing;

  // current time
  static std::string now()
  {
    time_t t = time(0);
    return std::string(ctime(&t));
  }

  // number of digits in m
  static int width(unsigned int m)
  {
    int w = 1;
    while (m >= 10) {
      m /= 10;
      w++;
    }
    return w;
  }
};

volatile bool MyProgress::_exit = false;

// derived graph class with specialized reader and progress reports
class ChacoGraph : public Graph {
public:
  // constructor
  ChacoGraph() : Graph() {}

  // read chaco graph from file
  bool read(FILE* file);
};

// read graph from file in chaco format
bool
ChacoGraph::read(FILE* file)
{
  try {
    char line[0x10000];
    if (!fgets(line, sizeof(line), file))
      throw std::string("file is empty");

    // determine number of nodes, edges, and type of graph
    unsigned int nv, ne, fmt;
    bool weighted = false;
    switch (sscanf(line, "%u%u%u", &nv, &ne, &fmt)) {
      case 2:
        break;
      case 3:
        switch (fmt) {
          case 0:
            break;
          case 1:
            weighted = true;
            break;
          default:
            throw std::string("invalid graph format specifier");
        }
        break;
      default:
        throw std::string("invalid first line in graph file");
    }

    // read nodes and their neighbors
    for (Node::Index i = 1; i <= nv; i++) {
      if (insert_node() != i) {
        std::ostringstream ss;
        ss << i;
        throw std::string("cannot insert node" + ss.str());
      }
      do {
        if (!fgets(line, sizeof(line), file)) {
          std::ostringstream ss;
          ss << i;
          throw std::string("no data for node" + ss.str());
        }
      } while (line[0] == '%' || line[0] == '#');
      int n;
      for (unsigned int k = 0, j; sscanf(line + k, "%u%n", &j, &n) == 1; k += n) {
        double w = 1;
        if (weighted) {
          k += n;
          if (sscanf(line + k, "%lf%n", &w, &n) != 1) {
            std::ostringstream ss;
            ss << i;
            throw std::string("cannot read weight for node" + ss.str());
          }
        }
        if (!insert_arc(i, j, Float(w), Float(w)))
          throw std::string("arc insertion failed");
      }
    }

    // make sure graph is undirected
    Arc::Index a = directed();
    if (a) {
      Node::Index i = arc_source(a);
      Node::Index j = arc_target(a);
      std::ostringstream ss;
      ss << i << ", " << j;
      throw std::string("no arc (") + ss.str() + std::string("): graph is directed");
    }
  }
  catch (std::string message) {
    std::cerr << "ERROR: " << message << std::endl;
    return false;
  }

  return true;
}

//@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-------------------------------------------

// signal handler for catching CTRL-C
static void
handler(int sig)
{
  MyProgress::exit();
  signal(sig, SIG_DFL);
}

//@m-a-i-n---f-u-n-c-t-i-o-n---------------------------------------------------

int
main(int argc, char *argv[])
{
  Functional* functional = 0; // ordering functional
  uint iterations = 1;        // number of V cycles
  uint window = 2;            // initial window size
  uint period = 1;            // iterations between window increment
  uint seed = (uint)time(0);  // random number seed
  FILE* psfile = 0;           // PostScript file

  // parse command-line arguments
  try {
    switch (argc) {
      case 7:
        if (!(psfile = fopen(argv[6], "w")))
          throw std::string("cannot create PostScript file");
        /*FALLTHROUGH*/
      case 6:
        if (sscanf(argv[5], "%u", &seed) != 1)
          throw std::string("invalid seed");
        /*FALLTHROUGH*/
      case 5:
        if (sscanf(argv[4], "%u", &period) != 1)
          throw std::string("invalid period");
        /*FALLTHROUGH*/
      case 4:
        if (sscanf(argv[3], "%u", &window) != 1)
          throw std::string("invalid window");
        /*FALLTHROUGH*/
      case 3:
        if (sscanf(argv[2], "%u", &iterations) != 1)
          throw std::string("invalid number of iterations");
        /*FALLTHROUGH*/
      case 2:
        if (argv[1][1])
          throw std::string("invalid functional");
        switch (*argv[1]) {
          case 'h':
            functional = new FunctionalHarmonic();
            break;
          case 'g':
            functional = new FunctionalGeometric();
            break;
          case 's':
            functional = new FunctionalSMR();
            break;
          case 'a':
            functional = new FunctionalArithmetic();
            break;
          case 'r':
            functional = new FunctionalRMS();
            break;
          case 'm':
            functional = new FunctionalMaximum();
            break;
          default:
            throw std::string("invalid functional");
        }
        break;
      default:
        throw std::string("");
    }
  }
  catch (std::string message) {
    if (!message.empty())
      std::cerr << "ERROR: " << message << std::endl;
    std::cerr << "Usage: gecko <functional> [iterations [window [period [seed [psfile]]]]] <graph >permutation" << std::endl;
    std::cerr << "Functionals:" << std::endl;
    std::cerr << "  h: harmonic mean" << std::endl;
    std::cerr << "  g: geometric mean" << std::endl;
    std::cerr << "  s: square mean root" << std::endl;
    std::cerr << "  a: arithmetic mean (1-sum, mla)" << std::endl;
    std::cerr << "  r: root mean square (2-sum)" << std::endl;
    std::cerr << "  m: maximum (bandwidth)" << std::endl;
    return EXIT_FAILURE;
  }

  // set up signal handler to catch interrupts
  signal(SIGINT, handler);
  signal(SIGTERM, handler);

  // read graph
  ChacoGraph graph;
  if (!graph.read(stdin))
    return EXIT_FAILURE;

  // optionally create PostScript drawing
  PostScript* ps = psfile ? new PostScript(graph.nodes(), psfile) : 0;
  Drawing* drawing = ps ? new Drawing(ps) : 0;
  MyProgress* progress = new MyProgress(drawing);

  // order graph
  std::cerr << "s = " << seed << std::endl;
  graph.order(functional, iterations, window, period, seed, progress);
  delete functional;

  // close PostScript file
  if (drawing) {
    delete drawing;
    delete ps;
    fclose(psfile);
  }
  delete progress;

  // output position of each node in reordered graph
  for (Node::Index i = 1; i <= graph.nodes(); i++)
    std::cout << graph.rank(i) << std::endl;

  return EXIT_SUCCESS;
}
