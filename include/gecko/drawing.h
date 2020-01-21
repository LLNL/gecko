#ifndef GECKO_DRAWING_H
#define GECKO_DRAWING_H

#include "gecko/graph.h"
#include "gecko/device.h"

namespace Gecko {

class Graph;

// 2D graph drawing
class Drawing {
public:
  Drawing(Device* d) : device(d) {}
  void draw(const Graph* g);
private:
  Device* device;
};

}

#endif
