#include "main/main.h"

#include "os/system.h"
#include "main/mcctrl.h"

#include "prlib.h"

// Main functions
void mainStart(void)
{
	// Initialize systems
	mccReqInit();
	CdctrlInit();
	PrInitializeModule(DBufDc.draw01.zbuf1);
}
