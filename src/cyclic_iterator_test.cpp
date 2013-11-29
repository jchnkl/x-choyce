#include <iostream>
#include <vector>

#include "cyclic_iterator.hpp"

typedef std::vector<int> data_t;
typedef const_cyclic_iterator<data_t> iter_t;

int main(int argc, char ** argv)
{
  data_t intvec = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  iter_t iter(&intvec);

  for (int i = 0; i < 20; ++i) {
    std::cerr << "iter + " << i << ": " << *(iter + i) << std::endl;
  }

  for (int i = 0; i < 20; ++i) {
    std::cerr << "iter - " << i << ": " << *(iter - i) << std::endl;
  }

  for (int i = 0; i < 20; ++i) {
    iter += i;
    std::cerr << "iter += " << i << ": " << *(iter) << std::endl;
    iter -= i;
  }

  std::cerr << "new iter" << std::endl;
  iter = iter_t(&intvec);

  for (int i = 0; i < 20; ++i) {
    iter -= i;
    std::cerr << "iter -= " << i << ": " << *(iter) << std::endl;
    iter += i;
  }

  return 0;

  std::cerr << "new iter" << std::endl;
  iter = iter_t(&intvec);

  iter += 5;
  std::cerr << "iter += 5: " << *(iter) << std::endl;

  iter -= 4;
  std::cerr << "iter -= 4: " << *(iter) << std::endl;

  iter -= 1;
  std::cerr << "iter -= 1: " << *(iter) << std::endl;

  iter -= 1;
  std::cerr << "iter -= 1: " << *(iter) << std::endl;

  iter -= 1;
  std::cerr << "iter -= 1: " << *(iter) << std::endl;

  std::cerr << "new iter" << std::endl;
  iter = iter_t(&intvec);

  std::cerr << "iter: " << *(iter) << std::endl;

  iter -= 1;
  std::cerr << "iter -= 1: " << *(iter) << std::endl;

  iter += 1;
  std::cerr << "iter += 1: " << *(iter) << std::endl;

  iter += 9;
  std::cerr << "iter += 9: " << *(iter) << std::endl;

  iter += 4;
  std::cerr << "iter += 4: " << *(iter) << std::endl;

  iter += 7;
  std::cerr << "iter += 7: " << *(iter) << std::endl;

  return 0;
}
