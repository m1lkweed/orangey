# 🟠 orangey
Orangey is a library to generate high-quality random numbers in various forms and distributions, all contained within a single-file header.
## Functions
```c
void orangey_srand(orangey_ctx_t *rng, __uint128_t initstate, __uint128_t initseq);
```
Seeds the rng state contained at `rng`, similar to `srand()`.
```c
uint64_t orangey_rand(orangey_ctx_t *rng);
```
Generates a random number, similar to `rand()`.
```c
uint64_t orangey_rand_range(orangey_ctx_t *rng, uint64_t min, uint64_t max);
```
Generates a number in the range \[`min`, `max`\]
```c
void orangey_skip(orangey_ctx_t *rng, __uint128_t delta);
```
Skips the next `delta` values from the generator in `O(lb(delta))` time.
```c
double orangey_uniform_double_01(orangey_ctx_t *rng);
```
Generates a float in the range \[0, 1\) with uniform density.
```c
double orangey_all_doubles_01(orangey_ctx_t *rng);
```
Has an equal chance of generating any representable float in the range \[0, 1\). Biased towards lower values.
```c
double orangey_gaussian(orangey_ctx_t *rng);
```
Generates floats with standard gaussian density.
```c
uint64_t orangey_poisson(orangey_ctx_t *rng, double ev);
```
Generates floats with poisson density.
