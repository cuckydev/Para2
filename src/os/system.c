#include <eekernel.h>

#include <stddef.h>
#include <stdlib.h>

#include "os/mtc.h"

// Malloc
extern char _end __attribute__((section(".data")));
static size_t _end_addr = (size_t)&_end;

extern char _stack_size __attribute__((section(".data")));
static size_t _stack_size_addr = (size_t)&_stack_size;

size_t FullAllocAndFree(void)
{
	size_t size = (size_t)(0x1ffefe0 - (_end_addr + _stack_size_addr + 0x1000));
	void *ptr = malloc(size);
	free(ptr);
	return size;
}

void mallocInit(void)
{
	// Initialize heap
	size_t size = FullAllocAndFree();
	scePrintf("HEAP SIZE[%08x]\n", size);
}

// Entry point
int main(int argc, char *argv[])
{
	// Initialize malloc
	mallocInit();

	// System loop
	while (1)
	{
		// Initialize Mtc
		MtcInit();
	}
}
