/* random.h, a C header for fast and easy random number generation.

Version: 1.2
Author: Erik Fast (fasterik.net)
License: CC0

This library provides a fast 64 bit PRNG using the xoshiro256++ algorithm, plus
some helper functions for generating integers in a range, random floats and
doubles, and sampling a normal distribution.

The following page was used as a reference:

https://prng.di.unimi.it/

xoshiro256++ was chosen because it's fast, simple to implement and understand,
and seems to have good enough statistical properties for non-cryptographic
purposes.

If you only need to generate floating point numbers, consider using the sibling
library, random_float.h instead. It uses the ~15% faster algorithm xoshiro256+,
which is recommended by the authors for floating point but not for general
purpose random numbers.


API:

// Initialize the PRNG with a 64 bit seed
void random_seed(RandomState *state, uint64_t seed);

// Generate a random unsigned 64 bit integer
uint64_t random_u64(RandomState *state);

// Generate 0 <= x < range
uint64_t random_range(RandomState *state, uint64_t range);

// Generate lower <= x <= upper
int random_int(RandomState *state, int lower, int upper);

// Generate floating point 0 <= x < 1
float random_float_01(RandomState *state);
double random_double_01(RandomState *state);

// Generate floating point lower <= x < upper
float random_float(RandomState *state, float lower, float upper);
double random_double(RandomState *state, double lower, double upper);

// Sample a normal distribution with the given mean and standard deviation
float random_float_gaussian(RandomState *state, float mu, float sigma);
double random_gaussian(RandomState *state, double mu, double sigma);


Changelog:

1.2:
  - Added random_float_gaussian, renamed random_gaussian to random_double_gaussian.
1.1:
  - Switched from xoshiro256+ to xoshiro256++.
1.0:
  - Initial release.

*/

#ifndef RANDOM_H_INCLUDE
#define RANDOM_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>

typedef struct {
    uint64_t s[4];
} RandomState;

// SplitMix64 implementation based on the one by Sebastiano Vigna:
//     https://prng.di.unimi.it/splitmix64.c
// This is only used to seed the RNG.
static inline uint64_t random__split_mix_64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

static inline void random_seed(RandomState *state, uint64_t seed) {
    state->s[0] = (seed = random__split_mix_64(seed));
    state->s[1] = (seed = random__split_mix_64(seed));
    state->s[2] = (seed = random__split_mix_64(seed));
    state->s[3] = (seed = random__split_mix_64(seed));
}

static inline uint64_t random__rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

// xoshiro256++ implementation based on the one by David Blackman and Sebastiano Vigna:
//     https://prng.di.unimi.it/xoshiro256plusplus.c
static inline uint64_t random_u64(RandomState *state) {
    const uint64_t result = random__rotl(state->s[0] + state->s[3], 23) + state->s[0];
    const uint64_t t = state->s[1] << 17;

    state->s[2] ^= state->s[0];
    state->s[3] ^= state->s[1];
    state->s[1] ^= state->s[2];
    state->s[0] ^= state->s[3];
    state->s[2] ^= t;
    state->s[3] = random__rotl(state->s[3], 45);

    return result;
}

// Debiased modulo (Java's method) from
//     https://www.pcg-random.org/posts/bounded-rands.html

// If you need a faster method, I suggest reading that page. I chose this one
// because it doesn't rely on compiler-specific details of 128 bit integers or
// bit manipulation intrinsics.
static inline uint64_t random_range(RandomState *state, uint64_t range) {
    uint64_t x, r;
    do {
        x = random_u64(state);
        r = x % range;
    } while (x - r > (-range));

    return r;
}

static inline int random_int(RandomState *state, int lower, int upper) {
    return lower + (int)random_range(state, upper - lower + 1);
}

static inline float random_float_01(RandomState *state) {
    return 0x1p-24f * (random_u64(state) >> 40);
}

static inline double random_double_01(RandomState *state) {
    return 0x1p-53 * (random_u64(state) >> 11);
}

static inline float random_float(RandomState *state, float lower, float upper) {
    return lower + (upper - lower) * random_float_01(state);
}

static inline double random_double(RandomState *state, double lower, double upper) {
    return lower + (upper - lower) * random_double_01(state);
}

static inline float random_float_gaussian(RandomState *state, float mu, float sigma) {
    // See https://en.wikipedia.org/wiki/Marsaglia_polar_method
    float u, s;
    do {
        u = random_float_01(state) * 2.0 - 1.0;
        s = u * u;
    } while (s >= 1.0 || s == 0.0);

    return mu + sigma * (u * sqrt(-0.5 * log(s) / s));
}

static inline double random_double_gaussian(RandomState *state, double mu, double sigma) {
    // See https://en.wikipedia.org/wiki/Marsaglia_polar_method
    double u, s;
    do {
        u = random_double_01(state) * 2.0 - 1.0;
        s = u * u;
    } while (s >= 1.0 || s == 0.0);

    return mu + sigma * (u * sqrt(-0.5 * log(s) / s));
}

#ifdef __cplusplus
}  //  extern "C"
#endif

#endif  //  RANDOM_H_INCLUDE

/*
To the extent possible under law, the author has dedicated all copyright and
related and neighboring rights to this software to the public domain worldwide.
This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>.
*/
