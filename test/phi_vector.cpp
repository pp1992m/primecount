///
/// @file  phi_vector.cpp
/// @brief Test that phi_vector(x, a) and phi(x, a)
///        results are identical
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primecount.hpp>
#include <generate.hpp>
#include <PiTable.hpp>
#include <phi_vector.hpp>

#include <stdint.h>
#include <iostream>
#include <random>
#include <vector>

using namespace std;
using namespace primecount;

int main()
{
  for (int i = 0; i < 100; i++)
  {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int64_t> dist(0, 1000000);

    int64_t x = dist(gen);
    int64_t y = isqrt(x) + 1000;

    PiTable pi(y);
    int64_t a = pi[y];

    auto primes = generate_primes<int64_t>(y);
    auto phi_vect = phi_vector(x, a, primes, pi, get_num_threads());

    for (int64_t i = 1; i < phi_vect.size(); i++)
    {
      int64_t phi1 = phi_vect[i];
      int64_t phi2 = phi(x, i - 1);

      if (phi1 != phi2)
      {
        cerr << "Error: phi_vector(x, i - 1) = " << phi1 << endl;
        cerr << "Correct: phi(x, i - 1) = " << phi2 << endl;
        cerr << "x = " << x << endl;
        cerr << "i - 1 = " << i - 1 << endl;
        cerr << "a = " << a << endl;
        exit(1);
      }
    }
  }

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
