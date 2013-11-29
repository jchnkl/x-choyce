#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <cmath>
#include <tuple>

namespace algorithm {

struct angle {
  double
  operator()(const std::tuple<int, int> & p1, const std::tuple<int, int> & p2)
  {
    double correction = 0;

    double dx = std::get<0>(p2) - std::get<0>(p1);
    double dy = std::get<1>(p2) - std::get<1>(p1);

    // http://www.mathsisfun.com/polar-cartesian-coordinates.html
    if (dx < 0) {
      // Q2 or Q3
      correction = M_PI;
    } else if (dy < 0) {
      // Q4
      correction = 2 * M_PI;
    }

    return correction + std::atan(dy / dx);
  }
}; // class angle

struct distance {
  double
  operator()(const std::tuple<int, int> & p1, const std::tuple<int, int> & p2)
  {
    // https://en.wikipedia.org/wiki/Cartesian_coordinate_system#Distance_between_two_points
    int dx = std::get<0>(p2) - std::get<0>(p1);
    int dy = std::get<1>(p2) - std::get<1>(p1);
    return std::sqrt(dx * dx + dy * dy);
  }
}; // class distance

}; // namespace algorithm

#endif // ALGORITHM_HPP
