#include <eekernel.h>
#include <sifrpc.h>
#include <libcdvd.h>
#include <devvif0.h>
#include <devvu0.h>
#include <libgraph.h>

#include <stdlib.h>

#include "os/mtc.h"
#include "os/syssub.h"

#include "macro.h"
#include "config.h"

// System globals
const char *iop_module[11] = {
	"cdrom0:\\IRX\\SIO2MAN.IRX;1",
	"cdrom0:\\IRX\\PADMAN.IRX;1",
	"cdrom0:\\IRX\\LIBSD.IRX;1",
	"cdrom0:\\IRX\\SDRDRV.IRX;1",
	"cdrom0:\\IRX\\MODMIDI.IRX;1",
	"cdrom0:\\IRX\\MODHSYN.IRX;1",
	"cdrom0:\\IRX\\MODMSIN.IRX;1",
	"cdrom0:\\IRX\\MCMAN.IRX;1",
	"cdrom0:\\IRX\\MCSERV.IRX;1",
	"cdrom0:\\IRX\\WAFE2PS2.IRX;1",
	"cdrom0:\\IRX\\TAPCTRL.IRX;1",
	"cdrom0:\\IRX\\SIO2MAN.IRX;1",
};

sceGsDBuffDc DBufDc;
int outbuf_idx = 0, oddeven_idx = 0;

sceGsDrawEnv1 *drawEnvP[5];
static sceGsDrawEnv1 drawEnvSp, drawEnvZbuff, drawEnvEnd;

static u_long128 GifPkCommon[0x2000];

// Forward declares
static void initSystem(void);
static void exitSystem(void);

// System control main thread
static void systemCtrlMain(void *user)
{
	return;
}

// Malloc
extern char _end __attribute__((section(".data")));
size_t _end_addr = (size_t)&_end;

extern char _stack_size __attribute__((section(".data")));
size_t _stack_size_addr = (size_t)&_stack_size;

size_t FullAllocAndFree(void)
{
	size_t size = (size_t)(0x1ffefe0 - (_end_addr + _stack_size_addr + 0x1000));
	void *ptr = malloc(size);
	free(ptr);
	return size;
}

static void mallocInit(void)
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
		// Initialize systems
		MtcInit();
		initSystem();
		
		// Start game
		MtcStart(systemCtrlMain);

		// Quit systems
		MtcQuit();
		exitSystem();
	}
}

// System
int SetIopModule(void)
{
	int i;

	// CD initialization
	sceSifInitRpc(0);

	sceCdInit(SCECdINIT);
	do {} while (sceSifRebootIop("cdrom0:\\IRX\\IOPRP23.IMG;1") == 0);

	do {} while (sceSifSyncIop() == 0);
	sceSifInitRpc(0);
	sceSifLoadFileReset();

	sceCdInit(SCECdINIT);
	sceCdMmode(SCECdCD);
	sceFsReset();
	sceSifInitIopHeap();

	// Load modules
	for (i = 0; i < MACRO_COUNTOF(iop_module); i++)
	{
		const char *module = iop_module[i];
		if (sceSifLoadModule(module) < 0)
		{
			printf("Can't load module [%s]\n", module);
			return 1;
		}
	}

	return 0;
}

static void firstClrFrameBuffer(void)
{
	// Setup packet
	struct
	{
		long p0 : 16; // TODO: What is this?
		long p1 : 8;
		long p2 : 8;
		long p3 : 32;
		sceGifTag giftag;
		sceGsDrawEnv1 draw;
		sceGsClear clear;
	} packet MACRO_AL16;

	packet.p0 = 15;
	packet.p2 = 0;
	packet.p3 = &packet.giftag;

	// SCE_GIF_CLEAR_TAG(&packet.giftag);
	packet.giftag.NLOOP = 14;
	packet.giftag.EOP = 1;
	packet.giftag.FLG = 0;
	packet.giftag.NREG = 1;
	packet.giftag.REGS0 = 14;

	packet.clear.rgbaq.Q = 1.0f;

	sceGsSetDefDrawEnv(&packet.draw, SCE_GS_PSMCT32, SCREEN_WIDTH, SCREEN_HEIGHT, SCE_GS_ZNOUSE, SCE_GS_ZNOUSE);
	sceGsSetDefClear(&packet.clear, SCE_GS_ZNOUSE, 2048 - (SCREEN_WIDTH >> 1), 2048 - (SCREEN_HEIGHT >> 1), SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0, 0);

	// Send packet
	sceDmaSync(sceDmaGetChan(SCE_DMA_GIF), 0, 0x7FFFFFFF);
	FlushCache(0);

	sceDmaSend(sceDmaGetChan(SCE_DMA_GIF), &packet);
	sceGsSyncPath(0, 0);
}

static void initSystem(void)
{
	// Read modules from CD
	SetIopModule();	

	// Initialize systems
	sceDevVif0Reset();
	sceDevVu0Reset();
	sceGsResetPath();
	sceDmaReset(1);

	// GPU init
	firstClrFrameBuffer();
	sceGsSyncV(0);
	sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME);

	// Initialize double buffer draw
	sceGsSetDefDBuffDc(&DBufDc, SCE_GS_PSMCT32, SCREEN_WIDTH, SCREEN_HEIGHT, SCE_GS_DEPTH_GEQUAL, SCE_GS_PSMZ32, 1);
	SetBackColor(0, 0, 0);

	*T0_MODE = T_MODE_CLKS_M | T_MODE_CUE_M;

	outbuf_idx = 0;

	FlushCache(0);
	sceGsSyncPath(0, 0);
	sceGsSwapDBuffDc(&DBufDc, outbuf_idx);

	// Initialize draw environments
	drawEnvP[0] = &DBufDc.draw01;
	drawEnvP[1] = &DBufDc.draw11;

	drawEnvSp = DBufDc.draw01;
	drawEnvSp.frame1.FBP = 0xD2;
	drawEnvP[2] = &drawEnvSp;
	sceGsSetHalfOffset(&drawEnvSp, 0x800, 0x800, 0);

	drawEnvZbuff = DBufDc.draw01;
	drawEnvZbuff.frame1.FBP = 0x8C;
	drawEnvP[3] = &drawEnvZbuff;
	sceGsSetHalfOffset(&drawEnvSp, 0x800, 0x800, 0);

	drawEnvEnd = DBufDc.draw01;
	drawEnvEnd.frame1.FBP = 0x140;
	drawEnvP[4] = &drawEnvEnd;
	sceGsSetHalfOffset(&drawEnvSp, 0x800, 0x800, 0);

	// Initialize common GIF
	CmnGifInit(GifPkCommon, MACRO_COUNTOF(GifPkCommon));
	CmnGifClear();
}

static void exitSystem(void)
{
}
