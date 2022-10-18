#ifndef PRLIB_H
#define PRLIB_H

#include <libgraph.h>

// PrLib public functions
#ifdef __cplusplus
extern "C"
{
#endif

void PrSetDebugParam(int i, int x);
void PrSetDebugParamFloat(float x, int i);
int PrGetDebugParam(int i);
float PrGetDebugParamFloat(int i);

void PrInitializeModule(sceGsZbuf zbuf);

#ifdef __cplusplus
}
#endif

#endif
