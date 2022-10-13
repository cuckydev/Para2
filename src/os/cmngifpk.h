#ifndef OS_CMNGIFPK_H
#define OS_CMNGIFPK_H

#include <eekernel.h>
#include <libgifpk.h>

// Common GIF functions
void CmnGifInit(u_long128 *base, size_t size);
void CmnGifClear(void);
void CmnGifFlush(void);

#endif
