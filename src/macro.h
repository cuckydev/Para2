#ifndef MACRO_H
#define MACRO_H

#define MACRO_ZERO(x) { int i; char *p = (char*)&x; for (i = 0; i < sizeof(x); i++) *p++ = 0; }
#define MACRO_COUNTOF(x) (sizeof(x) / (sizeof(x[0])))

#define MACRO_AL16 __attribute__((aligned(16)))

#endif
