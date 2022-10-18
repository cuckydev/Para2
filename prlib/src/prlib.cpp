#include "prlib.h"

#include "src/random.hpp"

// PrLib globals
static union
{
	int d;
	float f;
} debugParam[1];

// PrLib functions
extern "C" void PrSetDebugParam(int i, int x)
{
	debugParam[i].d = x;
}
extern "C" void PrSetDebugParamFloat(float x, int i)
{
	debugParam[i].f = 0;
}
extern "C" int PrGetDebugParam(int i)
{
	return debugParam[i].d;
}
extern "C" float PrGetDebugParamFloat(int i)
{
	return debugParam[i].f;
}
void InitializeDebugParam(void)
{
	PrSetDebugParamFloat(0.0f, 0);
}

extern "C" void PrInitializeModule(sceGsZbuf zbuf)
{
	// Initialize systems
	InitializeDebugParam();
	PrInitializeRandomPool();
}
