#include <stdio.h>

#define ORANGEY_IMPLEMENTATION
#include "orangey.h"

int main(){
	orangey_ctx_t rng = {};
	void orangey_srand(&rng, 0, 0);
	for(int i = 0; i < 100; ++i){
		printf("orangey_rand(&rng):                %.16lx\n", orangey_rand(&rng));
	}
	orangey_skip(&rng, -100);
	for(int i = 0; i < 100; ++i){
		printf("orangey_rand_range(&rng, 64, 128): %.16lx\n", orangey_rand_range(&rng, 64, 128));
	}
	orangey_skip(&rng, -100);
	for(int i = 0; i < 100; ++i){
		printf("orangey_uniform_double_01(&rng):   %.16lx\n", orangey_uniform_double_01(&rng));
	}
	orangey_skip(&rng, -100);
	for(int i = 0; i < 100; ++i){
		printf("orangey_all_doubles_01(&rng):      %.16lx\n", orangey_all_doubles_01(&rng));
	}
	orangey_skip(&rng, -100);
	for(int i = 0; i < 100; ++i){
		printf("orangey_gaussian(&rng):            %.16lx\n", orangey_gaussian(&rng));
	}
	orangey_skip(&rng, -100);
	for(int i = 0; i < 100; ++i){
		printf("orangey_poisson(&rng, 0.33333333): %.16lx\n", orangey_poisson(&rng, 0.33333333));
	}
}
