#include "bgm_i.h"
#include <bgmcmd.h>

#include "bgm_play.h"

extern void EnableIntr(int);

// RPC arguments
int gRpcArg[16];

// Bgm thread
static void *bgmFunc(unsigned int command, void *data, int size);

int sce_bgm_loop(void)
{
	sceSifQueueData rpc_qd;
	sceSifServeData rpc_sd;

	// Initialize interrupts
	CpuEnableIntr();
	EnableIntr(0x24);
	EnableIntr(0x28);
	EnableIntr(0x09);

	// Initialize RPC
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&rpc_qd, GetThreadId());
	sceSifRegisterRpc(&rpc_sd, sce_BGM_DEV, bgmFunc, (void*)gRpcArg, NULL, NULL, &rpc_qd);

	return 0;
}

// Bgm function
int ret = 0;
static void *bgmFunc(unsigned int command, void *data, int size)
{
	// ret = 0;

	switch (command)
	{
		case rBgmInit:
			ret = BgmInit(((int*)data)[0]);
			break;
		case rBgmQuit:
			BgmQuit();
			break;
		case rBgmOpen:
			ret = BgmOpen((const char*)data);
			break;
		case rBgmClose:
		case rBgmPreLoad:
		case rBgmStart:
		case rBgmStop:
		case rBgmSeek:
		case rBgmSetVolume:
		case rBgmSetVolumeDirect:
		case rBgmSetMasterVolume:
		case rBgmSetMode:
		case rBgmGetMode:
		case rBgmSdInit:
		case rBgmSetChannel:
		case rBgmCdInit:
		case rBgmGetTime:
		case rBgmGetTSample:
		case rBgmGetCdErrCode:
		case rBgmOpenFLoc:
		case rBgmSeekFLoc:
		case rBgmPreLoadBack:
		case rBgmSetTrPoint:
		case rBgmReadBuffFull:
		default:
			printf("EzBGM driver error: unknown command %d \n", ((int*)data)[0]);
			break;
	}

	return (void*)&ret;
}
