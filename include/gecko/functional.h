#ifndef GECKO_FUNCTIONAL_H
#define GECKO_FUNCTIONAL_H

#include <algorithm>
#include <cmath>
#include <vector>
#include "gecko/types.h"

namespace Gecko {

// abstract base class for weighted terms and sums
class WeightedValue {
public:
  WeightedValue(Float value, Float weight) : value(value), weight(weight) {}
  Float value;
  Float weight;
};

// weighted sum of terms
class WeightedSum : public WeightedValue {
public:
  WeightedSum(Float value = 0, Float weight = 0) : WeightedValue(value, weight) {}
};

// abstract base class for ordering functionals
class Functional {
public:
  virtual ~Functional() {}

  virtual WeightedSum sum(const WeightedSum& s, const WeightedValue& t) const
  {
    WeightedSum tot = s;
    accumulate(tot, sum(t));
    return tot;
  }

  virtual WeightedSum sum(const WeightedSum& s, const WeightedSum& t) const
  {
    WeightedSum tot = s;
    accumulate(tot, t);
    return tot;
  }

  // add weighted term to weighted sum
  virtual void accumulate(WeightedSum& s, const WeightedValue& t) const
  {
    accumulate(s, sum(t));
  }

  // add two weighted sums
  virtual void accumulate(WeightedSum& s, const WeightedSum& t) const
  {
    s.value += t.value;
    s.weight += t.weight;
  }

  // is s potentially less than t?
  virtual bool less(const WeightedSum& s, const WeightedSum& t) const
  {
    return s.value < t.value;
  }

  // transform term into weighted sum
  virtual WeightedSum sum(const WeightedValue& term) const = 0;

  // compute weighted mean from a weighted sum
  virtual Float mean(const WeightedSum& sum) const = 0;

  // compute k'th iteration bond for egde of length l and weight w
  virtual Float bond(Float w, Float l, uint k) const = 0;

  // compute position that minimizes weighted distance to a point set
  virtual Float optimum(const std::vector<WeightedValue>& v) const = 0;
};

// functionals with quasiconvex terms, e.g., p-means with p < 1.
class FunctionalQuasiconvex : public Functional {
protected:
  Float optimum(const std::vector<WeightedValue>& v, Float lmin) const
  {
    // Compute the optimum as the node position that minimizes the
    // functional.  Any nodes coincident with each candidate position
    // are excluded from the functional.
    Float x = v[0].value;
    Float min = GECKO_FLOAT_MAX;
    switch (v.size()) {
      case 1:
        // Only one choice.
        break;
      case 2:
        // Functional is the same for both nodes; pick node with
        // larger weight.
        if (v[1].weight > v[0].weight)
          x = v[1].value;
        break;
      default: 
        for (std::vector<WeightedValue>::const_iterator p = v.begin(); p != v.end(); p++) {
          WeightedSum s;
          for (std::vector<WeightedValue>::const_iterator q = v.begin(); q != v.end(); q++) {
            Float l = std::fabs(p->value - q->value);
            if (l > lmin)
              accumulate(s, WeightedValue(l, q->weight));
          }
          Float f = mean(s);
          if (f < min) {
            min = f;
            x = p->value;
          }
        }
        break;
    }
    return x;
  }
private:
  using Functional::optimum; // silence overload vs. override warning
};

// harmonic mean (p = -1)
class FunctionalHarmonic : public FunctionalQuasiconvex {
public:
  using Functional::sum;
  bool less(const WeightedSum& s, const WeightedSum& t) const
  {
    // This is only a loose bound when s.weight < t.weight.
    return s.value - s.weight > t.value - t.weight;
  }
  WeightedSum sum(const WeightedValue& term) const
  {
    return WeightedSum(term.weight / term.value, term.weight);
  }
  Float mean(const WeightedSum& sum) const
  {
    return sum.weight > 0 ? sum.weight / sum.value : 0;
  }
  Float bond(Float w, Float l, uint k) const
  {
    return w * std::pow(l, -Float(3) * Float(k) / Float(k + 1));
  }
  Float optimum(const std::vector<WeightedValue>& v) const
  {
    return FunctionalQuasiconvex::optimum(v, Float(0.5));
  }
};

// geometric mean (p = 0)
class FunctionalGeometric : public FunctionalQuasiconvex {
public:
  using Functional::sum;
  WeightedSum sum(const WeightedValue& term) const
  {
    return WeightedSum(term.weight * std::log(term.value), term.weight);
  }
  Float mean(const WeightedSum& sum) const
  {
    return sum.weight > 0 ? std::exp(sum.value / sum.weight) : 0;
  }
  Float bond(Float w, Float l, uint k) const
  {
    return w * std::pow(l, -Float(2) * Float(k) / Float(k + 1));
  }
  Float optimum(const std::vector<WeightedValue>& v) const
  {
    return FunctionalQuasiconvex::optimum(v, Float(0.5));
  }
};

// square mean root (p = 1/2)
class FunctionalSMR : public FunctionalQuasiconvex {
public:
  using Functional::sum;
  WeightedSum sum(const WeightedValue& term) const
  {
    return WeightedSum(term.weight * std::sqrt(term.value), term.weight);
  }
  Float mean(const WeightedSum& sum) const
  {
    return sum.weight > 0 ? (sum.value / sum.weight) * (sum.value / sum.weight) : 0;
  }
  Float bond(Float w, Float l, uint k) const
  {
    return w * std::pow(l, -Float(1.5) * Float(k) / Float(k + 1));
  }
  Float optimum(const std::vector<WeightedValue>& v) const
  {
    return FunctionalQuasiconvex::optimum(v, Float(0.0));
  }
};

// arithmetic mean (p = 1)
class FunctionalArithmetic : public Functional {
public:
  using Functional::sum;
  WeightedSum sum(const WeightedValue& term) const
  {
    return WeightedSum(term.weight * term.value, term.weight);
  }
  Float mean(const WeightedSum& sum) const
  {
    return sum.weight > 0 ? sum.value / sum.weight : 0;
  }
  Float bond(Float w, Float l, uint k) const
  {
    return w * std::pow(l, -Float(1) * Float(k) / Float(k + 1));
  }
  Float optimum(const std::vector<WeightedValue>& v) const
  {
    // Compute the optimum as the weighted median.  Since the median may
    // not be unique, the largest interval [x, y] is computed and its
    // centroid is chosen.  The optimum must occur at a node, and hence
    // we consider each node position pi at a time and the relative
    // positions of the remaining nodes pj.
    Float x = 0;
    Float y = 0;
    Float min = GECKO_FLOAT_MAX;
    for (std::vector<WeightedValue>::const_iterator p = v.begin(); p != v.end(); p++) {
      // Compute f = |sum_{j:pj<pi} wj - sum_{j:pj>pi} wj|.
      Float f = 0;
      for (std::vector<WeightedValue>::const_iterator q = v.begin(); q != v.end(); q++)
        if (q->value < p->value)
          f += q->weight;
        else if (q->value > p->value)
          f -= q->weight;
      f = std::fabs(f);
      // Update interval if f is minimal.
      if (f <= min) {
        if (f < min) {
          min = f;
          x = y = p->value;
        }
        else {
          x = std::min(x, p->value);
          y = std::max(y, p->value);
        }
      }
    }
    return (x + y) / 2;
  }
};

// root mean square (p = 2)
class FunctionalRMS : public Functional {
public:
  using Functional::sum;
  WeightedSum sum(const WeightedValue& term) const
  {
    return WeightedSum(term.weight * term.value * term.value, term.weight);
  }
  Float mean(const WeightedSum& sum) const
  {
    return sum.weight > 0 ? std::sqrt(sum.value / sum.weight) : 0;
  }
  Float bond(Float w, Float, uint) const
  {
    return w;
  }
  Float optimum(const std::vector<WeightedValue>& v) const
  {
    // Compute the optimum as the weighted mean.
    WeightedSum s;
    for (std::vector<WeightedValue>::const_iterator p = v.begin(); p != v.end(); p++) {
      s.value += p->weight * p->value;
      s.weight += p->weight;
    }
    return s.value / s.weight;
  }
};

// maximum (p = infinity)
class FunctionalMaximum : public Functional {
public:
  using Functional::sum;
  using Functional::accumulate;
  WeightedSum sum(const WeightedValue& term) const
  {
    return WeightedSum(term.value, term.weight);
  }
  void accumulate(WeightedSum& s, const WeightedSum& t) const
  {
    s.value = std::max(s.value, t.value);
  }
  Float mean(const WeightedSum& sum) const
  {
    return sum.value;
  }
  Float bond(Float, Float, uint) const
  {
    return Float(1);
  }
  Float optimum(const std::vector<WeightedValue>& v) const
  {
    // Compute the optimum as the midrange.
    Float min = v[0].value;
    Float max = v[0].value;
    for (std::vector<WeightedValue>::const_iterator p = v.begin() + 1; p != v.end(); p++) {
      if (p->value < min)
        min = p->value;
      else if (p->value > max)
        max = p->value;
    }
    return (min + max) / 2;
  }
};

}

#endif
