#ifndef GECKO_PROGRESS_H
#define GECKO_PROGRESS_H

#include <string>
#include "gecko/types.h"

namespace Gecko {

class Graph;

// Callbacks between iterations and phases.
class Progress {
public:
  virtual ~Progress() {}
  virtual void beginorder(const Graph* /*graph*/, Float /*cost*/) const {}
  virtual void endorder(const Graph* /*graph*/, Float /*cost*/) const {}
  virtual void beginiter(const Graph* /*graph*/, uint /*iter*/, uint /*maxiter*/, uint /*window*/) const {}
  virtual void enditer(const Graph* /*graph*/, Float /*mincost*/, Float /*cost*/) const {}
  virtual void beginphase(const Graph* /*graph*/, std::string /*name*/) const {};
  virtual void endphase(const Graph* /*graph*/, bool /*show*/) const {};
  virtual bool quit() const { return false; }
};

}

#endif
