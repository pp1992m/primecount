///
/// @file  pi_deleglise_rivat1.cpp
/// @brief Implementation of the Lagarias-Miller-Odlyzko prime counting
///        algorithm with the improvements of Deleglise and Rivat.
///        This implementation is based on src/lmo/pi_lmo5.cpp and the
///        paper: Tomás Oliveira e Silva, Computing pi(x): the
///        combinatorial method, Revista do DETUA, vol. 4, no. 6, March
///        2006, pp. 759-768.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primecount-internal.hpp>
#include <primesieve.hpp>
#include <bit_sieve.hpp>
#include <pmath.hpp>
#include <PhiTiny.hpp>
#include <tos_counters.hpp>

#include <stdint.h>
#include <algorithm>
#include <vector>

using namespace std;
using namespace primecount;

namespace {

/// Cross-off the multiples of prime in the sieve array.
/// For each element that is unmarked the first time update
/// the special counters tree data structure.
///
template <typename T1, typename T2>
void cross_off(int64_t prime, int64_t low, int64_t high, int64_t& next_multiple, T1& sieve, T2& counters)
{
  int64_t segment_size = sieve.size();
  int64_t k = next_multiple;

  for (; k < high; k += prime * 2)
  {
    if (sieve[k - low])
    {
      sieve.unset(k - low);
      cnt_update(counters, k - low, segment_size);
    }
  }
  next_multiple = k;
}

/// Calculate the contribution of the special leaves.
/// @see ../docs/computing-special-leaves.md
/// @pre y > 0 && c > 1
///
int64_t S2(int64_t x,
           int64_t y,
           int64_t z,
           int64_t pi_y,
           int64_t c,
           vector<int32_t>& primes,
           vector<int32_t>& lpf,
           vector<int32_t>& mu)
{
  int64_t limit = x / y + 1;
  int64_t segment_size = next_power_of_2(isqrt(limit));
  int64_t pi_sqrty = pi_bsearch(primes, isqrt(y));
  int64_t S2_result = 0;

  bit_sieve sieve(segment_size);
  vector<int32_t> counters(segment_size);
  vector<int32_t> pi = make_pi(y);
  vector<int64_t> next(primes.begin(), primes.end());
  vector<int64_t> phi(primes.size(), 0);

  // Segmented sieve of Eratosthenes
  for (int64_t low = 1; low < limit; low += segment_size)
  {
    // Current segment = interval [low, high[
    int64_t high = min(low + segment_size, limit);
    int64_t b = 2;

    sieve.fill(low);

    // phi(y, b) nodes with b <= c do not contribute to S2, so we
    // simply sieve out the multiples of the first c primes
    for (; b <= c; b++)
    {
      int64_t k = next[b];
      for (int64_t prime = primes[b]; k < high; k += prime * 2)
        sieve.unset(k - low);
      next[b] = k;
    }

    // Initialize special tree data structure from sieve
    cnt_finit(sieve, counters, segment_size);

    // For c + 1 <= b < pi_y
    // Find all special leaves: n = primes[b] * m, with mu[m] != 0 and primes[b] < lpf[m]
    // which satisfy: low <= (x / n) < high
    for (; b < pi_sqrty; b++)
    {
      int64_t prime = primes[b];
      int64_t min_m = max(x / (prime * high), y / prime);
      int64_t max_m = min(x / (prime * low), y);

      if (prime >= max_m)
        goto next_segment;

      for (int64_t m = max_m; m > min_m; m--)
      {
        if (mu[m] != 0 && prime < lpf[m])
        {
          int64_t n = prime * m;
          int64_t count = cnt_query(counters, (x / n) - low);
          int64_t phi_xn = phi[b] + count;
          S2_result -= mu[m] * phi_xn;
        }
      }

      phi[b] += cnt_query(counters, (high - 1) - low);
      cross_off(prime, low, high, next[b], sieve, counters);
    }

    // For pi_sqrty <= b < pi_y
    // Find all special leaves: n = primes[b] * primes[l]
    // which satisfy: low <= (x / n) < high
    for (; b < pi_y; b++)
    {
      int64_t prime = primes[b];
      int64_t l = pi[min(x / (prime * low), y)];

      if (prime >= primes[l])
        goto next_segment;

      int64_t min_m = max(x / (prime * high), y / prime);
      min_m = in_between(prime, min_m, y);
      int64_t min_trivial_leaf = pi[min(x / (prime * prime), y)];
      int64_t min_easy_leaf = pi[min(z / prime, y)];
      int64_t min_hard_leaf = pi[min_m];

      min_trivial_leaf = max(min_hard_leaf, min_trivial_leaf);
      min_easy_leaf = max(min_hard_leaf, min_easy_leaf);

      // For max(x / primes[b]^2, primes[b]) < primes[l] <= y
      // Find all trivial leaves which satisfy:
      // phi(x / (primes[b] * primes[l]), b - 1) = 1
      if (l > min_trivial_leaf)
      {
        S2_result += l - min_trivial_leaf;
        l = min_trivial_leaf;
      }

      // For max(z / primes[b], primes[b]) < primes[l] <= x / primes[b]^2
      // Find all easy leaves: n = primes[b] * primes[l]
      // x / n <= y such that phi(x / n, b - 1) = pi[x / n] - b + 2
      for (; l > min_easy_leaf; l--)
      {
        int64_t n = prime * primes[l];
        int64_t xn = x / n;
        S2_result += pi[xn] - b + 2;
      }

      // For max(x / (primes[b] * high), primes[b]) < primes[l] <= z / primes[b]
      // Find all hard leaves which satisfy:
      // low <= (x / n) < high
      for (; l > min_hard_leaf; l--)
      {
        int64_t n = prime * primes[l];
        int64_t xn = x / n;
        int64_t count = cnt_query(counters, xn - low);
        int64_t phi_xn = phi[b] + count;
        S2_result += phi_xn;
      }

      phi[b] += cnt_query(counters, (high - 1) - low);
      cross_off(prime, low, high, next[b], sieve, counters);
    }

    next_segment:;
  }

  return S2_result;
}

} // namespace

namespace primecount {

/// Calculate the number of primes below x using the
/// Deleglise-Rivat algorithm.
/// Run time: O(x^(2/3) / log x) operations, O(x^(1/3) * log log x) space.
///
int64_t pi_deleglise_rivat1(int64_t x)
{
  if (x < 2)
    return 0;

  // alpha is a tuning factor
  double d = (double) x;
  double alpha = in_between(1, log(d) - 3 * log(log(d)), iroot<6>(x));
  int64_t x13 = iroot<3>(x);
  int64_t y = (int64_t) (x13 * alpha);
  int64_t z = x / (int64_t) (x13 * sqrt(alpha));

  vector<int32_t> mu = make_moebius(y);
  vector<int32_t> lpf = make_least_prime_factor(y);
  vector<int32_t> primes;
  primes.push_back(0);
  primesieve::generate_primes(y, &primes);    

  int64_t pi_y = primes.size() - 1;
  int64_t c = min<int64_t>(PhiTiny::MAX_A, pi_y);
  int64_t s1 = S1(x, y, c, primes, lpf , mu);
  int64_t s2 = S2(x, y, z, pi_y, c, primes, lpf , mu);
  int64_t p2 = P2(x, y, 1);
  int64_t phi = s1 + s2;
  int64_t sum = phi + pi_y - 1 - p2;

  return sum;
}

} // namespace primecount
