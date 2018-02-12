# C++ classes

* **PrimeSieve** coordinates prime sieving, it instantiates
  the PreSieve, SievingPrimes and PrimeGenerator classes which do
  the actual prime sieving. PrimeSieve's main method is
  ```PrimeSieve::sieve(start, stop)``` which sieves the primes inside
  the interval [start, stop]. libprimesieve's C and C++ APIs are
  simple wrappers around the PrimeSieve class.

* **ParallelPrimeSieve** launches multiple threads using
  ```std::async``` and each thread sieves a part of the interval
  [start, stop] using a PrimeSieve object. At the end all partial
  results are combined to get the final result.

* **SieveOfEratosthenes** is an implementation of the segmented
  sieve of Eratosthenes using a bit array with 30 numbers per
  byte, each byte of the sieve array holds the 8 offsets
  ```k = { 7, 11, 13, 17, 19, 23, 29, 31 }```. Its main methods are
  ```addSievingPrime(prime)``` which is called consecutively for
  all primes ≤ sqrt(n) and ```sieve()``` which sieves the primes
  inside [start, stop]. SieveOfEratosthenes uses the EratSmall,
  EratMedium and EratBig classes to cross-off multiples.

* **CpuInfo** is used to get the CPU's L1 and L2 cache sizes. The
  best prime sieving performance is achieved using a sieve array
  size that matches the CPU's L1 or L2 cache size (depending on the
  CPU type).

* **PrimeGenerator** is derived from SieveOfEratosthenes. After a
  segment has been sieved (using SieveOfEratosthenes) PrimeGenerator
  is used to reconstruct primes and prime k-tuplets from 1 bits of
  the sieve array. It is also used for counting primes and prime
  k-tuplets in the sieve array.

* **SievingPrimes** is derived from SieveOfEratosthenes. The
  SievingPrimes class is used to generates the sieving
  primes ≤ sqrt(stop) for PrimeGenerator.

* **PreSieve** is used to pre-sieve multiples of small primes ≤ 19
  to speed up the sieve of Eratosthenes. Upon creation the
  multiples of small primes are removed from a buffer. Later this
  buffer is simply copied to the sieve array to remove (pre-sieve)
  the multiples of small primes.

* **Wheel** factorization is used to skip multiples of small primes
  ≤ 7 to speed up the sieve of Eratosthenes. The abstract
  **Wheel** class is used to initialize sieving primes i.e.
  ```Wheel::addSievingPrime()``` calculates the first
  multiple >= start of each sieving prime and the position
  within the SieveOfEratosthenes array of that
  multiple. ```Wheel::unsetBit()``` is used to cross-off a multiple
  (unset a bit) and to calculate the sieving prime's next multiple.

* **EratSmall** is derived from Wheel. EratSmall is a segmented
  sieve of Eratosthenes algorithm with a hard-coded modulo 30 wheel
  that skips multiples of 2, 3 and 5. This algorithm is optimized
  for small sieving primes that have many multiples in each
  segment. EratSmall is a further optimized implementation of Achim
  Flammenkamp's algorithm
  [[1]](https://github.com/kimwalisch/primesieve/tree/master/src#references).

* **EratMedium** is derived from Wheel. EratMedium is a segmented
  sieve of Eratosthenes algorithm with a modulo 210 wheel that skips
  multiples of 2, 3, 5 and 7. The wheel is implemented using a
  precomputed lookup table (```wheel210``` array from
  ```Wheel.cpp```). This algorithm is optimized for medium sieving
  primes with a few multiples per segment.

* **EratBig** is derived from Wheel. EratBig is a segmented sieve of
  Eratosthenes algorithm with Tomás Oliveira's improvement for big
  sieving primes [[2]](https://github.com/kimwalisch/primesieve/tree/master/src#references)
  and a modulo 210 wheel that skips multiples of 2, 3, 5 and 7. The
  wheel is implemented using a precomputed lookup table (```wheel210```
  array from ```Wheel.cpp```). EratBig is optimized for big sieving
  primes that have less than one multiple per segment.

* **primesieve::iterator** allows to easily iterate over primes. It
  provides ```next_prime()``` and ```prev_prime()```
  methods. primesieve::iterator works using a cache of primes of
  size sqrt(stop), once there are no more primes available in the
  cache primesieve::iterator will generate new primes and refill the
  cache.

# References

1. Achim Flammenkamp, "The Art of Prime Sieving", 1998. <br/>
   http://wwwhomes.uni-bielefeld.de/achim/prime_sieve.html
2. Tomás Oliveira e Silva, "Fast implementation of the segmented
   sieve of Eratosthenes", 2002. <br/>
   http://www.ieeta.pt/~tos/software/prime_sieve.html