#ifndef OS_SYSSUB_H
#define OS_SYSSUB_H

#include <stddef.h>

// Syssub types
struct Pad
{
	
};

// Syssub functions
void WorkClear(void *ptr, size_t size);

void GPadInit(void);
void GPadExit(void);
void GPadSysRead(void);
void GPadRead(void);

void SetBackColor(int r, int g, int b);

int randMakeMax(int max);

char *ByteStringSub(char *s, int v);
char *ByteString(int v);
void ReportHeapUsage(void);

void usrMallcInit(void);
void *usrMalloc(size_t size);
void usrFree(void *adr);
void usrMallcReport(void);

#endif
