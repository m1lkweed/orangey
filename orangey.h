#ifndef ORANGEY_H
#define ORANGEY_H
#include <stddef.h>
#include <stdint.h>

#define ORANGEY_CONSTANT_128(high,low) ((((__uint128_t)high) << 64) + low)

typedef struct{
	__uint128_t state;
	__uint128_t inc;
}orangey_ctx_t;

void orangey_srand(orangey_ctx_t *rng, __uint128_t initstate, __uint128_t initseq);
uint64_t orangey_rand(orangey_ctx_t *rng);
uint64_t orangey_rand_range(orangey_ctx_t *rng, uint64_t _min, uint64_t _max);
void orangey_skip(orangey_ctx_t *rng, __uint128_t delta);
double orangey_uniform_double_01(orangey_ctx_t *rng);
double orangey_all_doubles_01(orangey_ctx_t *rng);
double orangey_gaussian(orangey_ctx_t *rng);
uint64_t orangey_poisson(orangey_ctx_t *rng, double ev);

#ifdef ORANGEY_IMPLEMENTATION
#include <math.h>
inline uint64_t orangey_rotr(uint64_t value, unsigned int rot){
	return (value >> rot) | (value << ((-rot) & 63));
}

inline uint64_t orangey_output(__uint128_t state){
	return orangey_rotr(((uint64_t)(state >> 64u)) ^ (uint64_t)state, state >> 122u);
}

#define ORANGEY_MUL ORANGEY_CONSTANT_128(2549297995355413924ULL, 4865540595714422341ULL)
#define ORANGEY_INC  ORANGEY_CONSTANT_128(6364136223846793005ULL, 1442695040888963407ULL)
#define ORANGEY_STATE_INIT ORANGEY_CONSTANT_128(0x979c9a98d8462005ULL, 0x7d3e9cb6cfe0549bULL)

inline __uint128_t orangey_advance_lcg_128(__uint128_t state, __uint128_t delta, __uint128_t cur_mult, __uint128_t cur_plus){
	__uint128_t acc_mult = 1u;
	__uint128_t acc_plus = 0u;
	while (delta > 0) {
		if(delta & 1){
			acc_mult *= cur_mult;
			acc_plus = acc_plus * cur_mult + cur_plus;
		}
		cur_plus = (cur_mult + 1) * cur_plus;
		cur_mult *= cur_mult;
		delta /= 2;
	}
	return acc_mult * state + acc_plus;
}

inline void orangey_step(orangey_ctx_t *rng){
	rng->state = rng->state * ORANGEY_MUL + rng->inc;
}

void orangey_skip(orangey_ctx_t *rng, __uint128_t delta){
	rng->state = orangey_advance_lcg_128(rng->state, delta, ORANGEY_MUL, rng->inc);
}

void orangey_srand(orangey_ctx_t *rng, __uint128_t initstate, __uint128_t initseq){
	rng->state = 0U;
	rng->inc = (initseq << 1u) | 1u;
	orangey_step(rng);
	rng->state += initstate;
	orangey_step(rng);
}

uint64_t orangey_rand(orangey_ctx_t *rng){
	orangey_step(rng);
	return orangey_output(rng->state);
}

uint64_t orangey_rand_range(orangey_ctx_t *rng, uint64_t _min, uint64_t _max){
	uint64_t max = (_min > _max)?_min:_max;
	uint64_t min = (_min < _max)?_min:_max;
	uint64_t range = max - min;
	if(max == min)
		return max;
	if(__builtin_popcountl(range) == 1)
		return (orangey_rand(rng) & (range - 1)) + min;
	uint64_t limit = (uint64_t)-(int64_t)range % range, r;
	do
		r = orangey_rand(rng);
	while(r < limit);
	r %= range;
	return r + min;
}

// Doesn't represent all doubles; not biased towards smaller values
// This is usually what you want
double orangey_uniform_double_01(orangey_ctx_t *rng){
	const uint64_t mask = 0x000FFFFFFFFFFFFFUL;
	const uint64_t sExp = 0x3FF0000000000000UL;
	union{
		uint64_t as_int;
		double as_double;
	}rnd = {
		.as_int = orangey_rand(rng)
	};
	rnd.as_int &= mask;
	rnd.as_int |= sExp;
	return rnd.as_double - 1;
}

// Equal chance of hitting any representable number in [0, 1]
// Biased towards smaller values
double orangey_all_doubles_01(orangey_ctx_t *rng){
	int exponent = -64;
	uint64_t significand;
	unsigned shift;
	do{
		exponent -= 64;
		if(exponent < -1074){
			return 0;
		}
	}while((significand = orangey_rand(rng)) == 0);
	shift = __builtin_clzl(significand);
	if(shift != 0){
		exponent -= shift;
		significand <<= shift;
		significand |= (orangey_rand(rng) >> (64 - shift));
	}
	significand |= 1;
	return ldexp((double)significand, exponent);
}

double orangey_gaussian(orangey_ctx_t *rng){
	double rsq;
	do{
		rsq = orangey_uniform_double_01(rng);
	}while(rsq == 0.0);
	return orangey_uniform_double_01(rng) * sqrt(-2.0 * log(rsq) / rsq);
}

uint64_t orangey_poisson(orangey_ctx_t *rng, double ev){
	uint64_t n = 0;
	double em = exp(-ev);
	double x = orangey_uniform_double_01(rng);
	while(x > em){
		++n;
		x *= orangey_uniform_double_01(rng);
	}
	return n;
}

/* Orangey is based on:
 * https://github.com/imneme/pcg-c (Dual-licensed under MIT or Apache 2.0)
 * https://emunix.emich.edu/~shaynes/Papers/ProbabilityDistributions/probabilityDistributions.html (Unlicensed)
 * http://mumble.net/~campbell/tmp/random_real.c (Permissively licensed, see below)
 * All other code (c) 2023 m1lkweed, licensed the same as pcg-c
 *
 *
 * All functions related to the random number state machine originally come from pcg-c under either (both?) of the following licenses:
 *
 * Copyright (c) 2014-2017 Melissa O'Neill and PCG Project contributors
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Apache license linked for brevity: https://github.com/imneme/pcg-c/blob/master/LICENSE-APACHE.txt
 *
 *
 *
 * The functions orangey_gaussian() and orangey_poisson() were modified from Susan Haynes' code.
 * Her functions came with no license and were admittedly copied from:
 * Press, et al., Numerical Recipes in C: The Art of Scientific Computing, Cambridge University Press, 2nd ed, 1992.
 * The functions above are modified and use much more straight-forward math,
 * hopefully to the point of not infringing on any possible rights.
 *
 *
 *
 * The random_real.c function from which orangey_all_doubles_01() was modified contained the following notice:
 *
 *    Copyright (c) 2014, Taylor R Campbell
 *
 *    Verbatim copying and distribution of this entire article are
 *    permitted worldwide, without royalty, in any medium, provided
 *    this notice, and the copyright notice, are preserved.
 *
 * It's unclear if this "license" permits modification and/or incomplete copies, but no other functions in the header rely on this code.
 */
 #endif
 #endif
