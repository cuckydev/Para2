#include "os/cmngifpk.h"

#include "macro.h"

// Common GIF globals
struct CmnGifPri
{
	int w0, w1;
};
static struct CmnGifPri cmngif_pri[64];

static u_long128 cmnGifTr[64];
static sceGifPacket cmnGifPacket;

static int cmngif_pri_cnt;
static u_long128 *cmnGifPkBase, *cmnGifPkCurrent, *cmnGifPkEnd;

// Common GIF functions
void CmnGifInit(u_long128 *base, size_t size)
{
	// Set pointers
	cmnGifPkEnd = base + size;
	cmnGifPkBase = base;
	cmnGifPkCurrent = base;

	// Set and clear packet area
	sceGifPkInit(&cmnGifPacket, cmnGifTr);
	CmnGifClear();
}

void CmnGifClear(void)
{
	int i;

	// Reset pointer
	cmnGifPkCurrent = cmnGifPkBase;

	// Clear packets
	for (i = (MACRO_COUNTOF(cmngif_pri) - 1); i >= 0; i--)
		cmngif_pri[i].w1 = 0;
	cmngif_pri_cnt = 0;

	// Reset packet
	sceGifPkReset(&cmnGifPacket);
}

void CmnGifFlush(void)
{
	
}
