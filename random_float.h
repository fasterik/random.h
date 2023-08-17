/* random_float.h, a C header for fast and easy floating point number generation.

Version: 1.0
Author: Erik Fast (fasterik.net)
License: CC0

This library provides fast floating point number generation using the
xoshiro256+ algorithm. If you need a more general purpose random number
generator, consider using the sibling library, random.h.

The following page was used as a reference:

https://prng.di.unimi.it/


API:

// Initialize the PRNG with a 64 bit seed
void rf_seed(RFState *state, uint64_t seed);

// Generate 0 <= x < 1
float rf_float_01(RFState *state);
double rf_double_01(RFState *state);

// Generate lower <= x < upper
float rf_float(RFState *state, float lower, float upper);
double rf_double(RFState *state, double lower, double upper);

// Sample a normal distribution with the given mean and standard deviation
float rf_float_gaussian(RFState *state, float mu, float sigma);
double rf_double_gaussian(RFState *state, double mu, double sigma);


Changelog:

1.0:
  - Initial release.

*/

#ifndef RANDOM_FLOAT_H_INCLUDE
#define RANDOM_FLOAT_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>

typedef struct {
    uint64_t s[4];
} RFState;

// SplitMix64 implementation based on the one by Sebastiano Vigna:
//     https://prng.di.unimi.it/splitmix64.c
// This is only used to seed the RNG.
static inline uint64_t rf__split_mix_64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

static inline void rf_seed(RFState *state, uint64_t seed) {
    state->s[0] = (seed = rf__split_mix_64(seed));
    state->s[1] = (seed = rf__split_mix_64(seed));
    state->s[2] = (seed = rf__split_mix_64(seed));
    state->s[3] = (seed = rf__split_mix_64(seed));
}

// xoshiro256+ implementation based on the one by David Blackman and Sebastiano Vigna:
//     https://prng.di.unimi.it/xoshiro256plus.c
static inline uint64_t rf__next(RFState *state) {
    const uint64_t result = state->s[0] + state->s[3];
    const uint64_t t = state->s[1] << 17;

    state->s[2] ^= state->s[0];
    state->s[3] ^= state->s[1];
    state->s[1] ^= state->s[2];
    state->s[0] ^= state->s[3];
    state->s[2] ^= t;
    state->s[3] = (state->s[3] << 45) | (state->s[3] >> 19);

    return result;
}

static inline float rf_float_01(RFState *state) {
    return 0x1p-24f * (rf__next(state) >> 40);
}

static inline double rf_double_01(RFState *state) {
    return 0x1p-53 * (rf__next(state) >> 11);
}

static inline float rf_float(RFState *state, float lower, float upper) {
    return lower + (upper - lower) * rf_float_01(state);
}

static inline double rf_double(RFState *state, double lower, double upper) {
    return lower + (upper - lower) * rf_double_01(state);
}

static inline float rf_float_gaussian(RFState *state, float mu, float sigma) {
    // See https://en.wikipedia.org/wiki/Marsaglia_polar_method
    float u, s;
    do {
        u = rf_float_01(state) * 2.0 - 1.0;
        s = u * u;
    } while (s >= 1.0 || s == 0.0);

    return mu + sigma * (u * sqrt(-0.5 * log(s) / s));
}

static inline double rf_double_gaussian(RFState *state, double mu, double sigma) {
    // See https://en.wikipedia.org/wiki/Marsaglia_polar_method
    double u, s;
    do {
        u = rf_double_01(state) * 2.0 - 1.0;
        s = u * u;
    } while (s >= 1.0 || s == 0.0);

    return mu + sigma * (u * sqrt(-0.5 * log(s) / s));
}

#ifdef __cplusplus
}  //  extern "C"
#endif

#endif  //  RANDOM_FLOAT_H_INCLUDE

/*
To the extent possible under law, the author has dedicated all copyright and
related and neighboring rights to this software to the public domain worldwide.
This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>.
*/
