#ifndef GECKO_DEVICE_H
#define GECKO_DEVICE_H

#include "gecko/types.h"

namespace Gecko {

// drawing device abstract base class
class Device {
public:
  virtual ~Device() {}
  virtual void begin() {}
  virtual void end() {}
  virtual void node(Float x, Float r, Float gray = 0.5) = 0;
  virtual void edge(Float xi, Float xj, Float weight) = 0;
  virtual void edge(Float xi, Float xj, Float weight, bool top) = 0;
};

}

#endif
