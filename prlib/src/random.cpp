#include "src/random.hpp"

#include "macro.h"

// Random globals
static int randomPool[0x61];

int randomSeed = 1;
int poolIndex = 0;

// Random functions
int RawRandom(void)
{
	randomSeed = randomSeed * 0x5D588B65 + 1;
	return randomSeed;
}

int PrRandom(void)
{
	poolIndex = randomPool[poolIndex] % MACRO_COUNTOF(randomPool);
	int result = randomPool[poolIndex];
	randomPool[poolIndex] = RawRandom();
	return result >> 1;
}

void PrInitializeRandomPool(void)
{
	for (int i = 0; i < MACRO_COUNTOF(randomPool); i++)
		randomPool[i] = RawRandom();
	PrRandom();
	PrRandom();
}
