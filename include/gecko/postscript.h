#ifndef GECKO_POSTSCRIPT_H
#define GECKO_POSTSCRIPT_H

#include <cstdio>
#include "gecko/device.h"

namespace Gecko {

// PostScript document
class PostScript : public Device {
public:
  PostScript(uint nodes, FILE* file = stdout, uint wx = 576, uint wy = 576) : file(file), page(1)
  {
    fprintf(file, "%%!PS-Adobe-3.0 EPSF-2.0\n");
    fprintf(file, "%%%%BoundingBox: 0 0 %u %u\n", wx, wy);
    fprintf(file, "100 dict begin\n");
    fprintf(file, "/Q { setrgbcolor rectfill } bind def\n");
    fprintf(file, "%% circle: x y r\n");
    fprintf(file, "/c { newpath 0 360 arc stroke } bind def\n");
    fprintf(file, "%% disc: x y r\n");
    fprintf(file, "/d { newpath 0 360 arc fill } bind def\n");
    fprintf(file, "%% node: xpos radius grayfill n\n");
    fprintf(file, "/n { 0.125 setlinewidth setgray 0 exch 3 copy d 0 setgray c } bind def\n");
    fprintf(file, "/a { newpath setlinewidth exch 0 moveto 0 lineto stroke } bind def\n");
    fprintf(file, "%% arc: x0 y0 x1 y1 x2 y2 r\n");
    fprintf(file, "/A { newpath setlinewidth 7 -2 roll moveto arct stroke } bind def\n");
    origin = 0.5 * wy;
    scale = double(wx) / nodes;
  }
  ~PostScript()
  {
    fprintf(file, "end\n");
    fprintf(file, "%%%%EOF\n");
  }
  void begin()
  {
    fprintf(file, "%%%%Page: %u\n", page);
    fprintf(file, "gsave\n");
    fprintf(file, "0 %g translate\n", origin);
    fprintf(file, "%g dup scale\n", scale);
    fprintf(file, "2 setlinejoin\n");
  }
  void end()
  {
    fprintf(file, "grestore\n");
    fprintf(file, "showpage\n");
    page++;
  }
  void node(Float x, Float r, Float gray = 0.5)
  {
    fprintf(file, "%g %g %g n\n", x, r, gray);
  }
  void edge(Float xi, Float xj, Float weight)
  {
    weight /= 4;
    fprintf(file, "%g %g %g a\n", xi, xj, weight);
  }
  void edge(Float xi, Float xj, Float weight, bool top)
  {
    double x = (xi + xj) / 2;
    double d = fabs(xj - xi) / 2;
    double h = std::min(d * d * (2 * d - 1), 1e5);
    double y = d * d / h;
    double r = sqrt(d * d + y * y);
    weight /= 4;
    if (top)
      fprintf(file, "%g 0 %g %g %g 0 %g %g A\n", xj, x, +h, xi, r, weight);
    else
      fprintf(file, "%g 0 %g %g %g 0 %g %g A\n", xi, x, -h, xj, r, weight);
  }
private:
  FILE* file;
  double origin;
  double scale;
  uint page;
};

}

#endif
