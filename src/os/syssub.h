#ifndef OS_SYSSUB_H
#define OS_SYSSUB_H

#include <stddef.h>

// Syssub types
#define PAD_L2 (1 << 0)
#define PAD_R2 (1 << 1)
#define PAD_L1 (1 << 2)
#define PAD_R1 (1 << 3)

#define PAD_TRIANGLE (1 << 4)
#define PAD_CIRCLE   (1 << 5)
#define PAD_CROSS    (1 << 6)
#define PAD_SQUARE   (1 << 7)

#define PAD_SELECT (1 << 8)
#define PAD_L3     (1 << 9)
#define PAD_R3     (1 << 10)
#define PAD_START  (1 << 11)

#define PAD_UP    (1 << 12)
#define PAD_RIGHT (1 << 13)
#define PAD_DOWN  (1 << 14)
#define PAD_LEFT  (1 << 15)

#define PAD_RX 0
#define PAD_RY 1
#define PAD_LX 2
#define PAD_LY 3

#define PAD_PRIGHT 0
#define PAD_PLEFT  1
#define PAD_PUP    2
#define PAD_PDOWN  3

#define PAD_PTRIANGLE 4
#define PAD_PCIRCLE   5
#define PAD_PCROSS    6
#define PAD_PSQUARE   7

#define PAD_PL1 8
#define PAD_PR1 9
#define PAD_PL2 10
#define PAD_PR2 11

struct Pad
{
	unsigned int status;
	unsigned short change, held, press, release;
	unsigned char a[4];
	unsigned char p[12];
	unsigned char act[2];
	unsigned short aheld, apress;
	unsigned short unk0;
};

// Syssub functions
void WorkClear(void *ptr, size_t size);

void GPadInit(void);
void GPadExit(void);
void GPadSysRead(void);
void GPadRead(struct Pad *pad);

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
